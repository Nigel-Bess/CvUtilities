#include "vmb-camera.h"

VmbCamera::VmbCamera(std::string ip, int bay, fulfil::utils::Logger* log, std::shared_ptr<GrpcService> serv): 
                camera_ip_(ip), bay_(bay), log_(log), service_(serv){
    SetName();
}

void VmbCamera::StartCamera(){
    std::thread(&VmbCamera::RunCamera, this).detach();
}

void VmbCamera::KillCamera(){
    run_ = false;
    log_->Info("VmbCamera shutdown on {}", name_);
    if(camera_ != nullptr)
        camera_->Close();
}

void VmbCamera::RunSetup(){
    log_->Info("VmbCamera starting on {}", name_);
    auto code = camera_->Open(VmbAccessModeFull);
    auto first = true;
    while(code != VmbErrorSuccess){
        log_->Error("{} returned {} when trying to open camera", name_, GetVimbaCode(code));
        if(first)
            AddCameraStatus(DepthCameras::DcCameraStatusCodes::CAMERA_STATUS_RECOVERABLE_EXCEPTION);
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
        code = camera_->Open(VmbAccessModeFull);
        first = false;
    }
    std::string name;
    camera_->GetName(name);
    log_->Info("{} [{}] open with FWv: {}]", name_, name, GetFeatureString("DeviceFirmwareID"));
    // SetFeature("ExposureTime", 15000.0);
    SetFeature("PixelFormat", "BGR8");
    SetFeature("ExposureAuto", "Once");
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));//wait for auto exp to kick in
    AdjustPacketSize();
    connected_ = true;
    AddCameraStatus(DepthCameras::DcCameraStatusCodes::CAMERA_STATUS_CONNECTED);
    GetImageBlocking();
    SaveLastImage(name_);
}
//sudo ifconfig enp65s0f0 mtu 9000
//sudo ifconfig enp65s0f1 mtu 9000
void VmbCamera::RunCamera(){
    while(run_){
        if(!connected_)RunSetup();
        else{
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            std::string name;
            VmbErrorType code;
            std::lock_guard<std::mutex> lock(_lock);{
                code = camera_->GetName(name);
            }
            // std::cout << name << std::endl;
            if(code != VmbErrorSuccess){
                log_->Error("{} returned {} when trying to poll camera, disconnecting", name_, GetVimbaCode(code));
                if(camera_ != nullptr)
                    camera_->Close();
                connected_ = false;
            }
        }
    }
}

void VmbCamera::AddCameraStatus(DepthCameras::DcCameraStatusCodes code){
    if(service_ == nullptr)return;
    log_->Info("{} Sending status code to FC [{}]", name_,  DcCameraStatusCodes_Name(code));
    DepthCameras::CameraStatusUpdate msg;
    std::string tostr;
    msg.set_command_id(code == 0 ? "cafebabecafebabecafebabe" : "deadbeefdeadbeefdeadbeef"); 
    msg.set_msg_type(DepthCameras::MESSAGE_TYPE_CAMERA_STATUS);
    msg.set_camera_name(name_);
    msg.set_camera_serial(camera_ip_);
    msg.set_status_code(code);
    msg.SerializeToString(&tostr);
    service_->AddStatusUpdate(msg.msg_type(), tostr, msg.command_id());
}

void VmbCamera::SaveLastImage(std::string path){
    try{
        std::string img_name = path + ".bmp";
        if(last_mat_.size().empty()){
            log_->Error("Cannot save emtpy image to {}", img_name);
            return;
        }
        cv::imwrite(img_name, last_mat_);
        log_->Info("{} saved successfully!!!", img_name);// img_name << " saved successfully!" << std::endl;

    }
    catch(const std::exception &ex){
        std::cout << ex.what() << std::endl;
    }
    catch(...){
        std::cout << "Error saving image" << std::endl;
    }
}

std::shared_ptr<cv::Mat> VmbCamera::GetImageBlocking(){
    VmbUint32_t height;
    VmbUint32_t width;
    auto empty = std::make_shared<cv::Mat>();
    if(!connected_){
        // log_->Error("Can't get image {} is disconnected", name_);
        return empty;
    }
    auto count = 0;
    auto err = VmbErrorCustom;
    
    std::lock_guard<std::mutex> lock(_lock);
    {
        while(err != VmbErrorSuccess || count < 1){
            err = camera_->AcquireSingleImage(frame_ptr_, 5000);
            
            if(err != VmbErrorSuccess)log_->Error("{} could not get frame with code {}", name_, GetVimbaCode(err));
            count++;
            if(count > 3)break;
        }
        if(err != VmbErrorSuccess){
            log_->Error("{} could not get frame with code {}", name_, GetVimbaCode(err));
            return empty;
        }
        try{
            auto val = frame_ptr_->GetImage(img_buffer_);
            frame_ptr_->GetHeight(height);
            frame_ptr_->GetWidth(width);
            // std::cout << "GOT FRAME " << height << " x " << width  << std::endl;
            last_mat_ = cv::Mat(height, width, CV_8UC3, img_buffer_);
            // std::cout << "Got mat of size" << last_mat_.size() << std::end;
            last_image_ = std::make_shared<cv::Mat>(last_mat_);
            return last_image_;
        }
        catch(const std::exception &ex){
            std::cout << ex.what() << std::endl;
        }
    }
    return empty;
}

std::string VmbCamera::GetMacAddress(){    
    VmbCPP::LocalDevicePtr local;
    VmbCPP::FeaturePtr feat;
    camera_->GetLocalDevice(local);
    local->GetFeatureByName("GevDeviceMACAddress", feat);    
    VmbInt64_t val;//
    feat->GetValue(val);
    uint32_t h = ((val >> 24) & 0xFFFFFF);
    uint32_t l = (val & 0xFFFFFF);
    char buff[14];
    sprintf(buff, "%06X%06X", h, l);
    std::string mac(buff);
    // local->GetFeatureByName("GevCurrentIPAddress", feat);   
    // feat->GetValue(val);
    // char ip[20];
    // for(auto i = 0; i < 4; i++){
    //     uint8_t addr = (int)((val >> i * 8) & 0xFF);
    //     sprintf(ip + i * 4, ".%03d", addr);

    // }
    // camera_ip_ = std::string(ip);
    // std::cout << name_ << camera_ip_ << std::endl;
    return mac;
}

void VmbCamera::SetFeature(std::string feature, std::string value){
    std::string fvalue = GetFeatureString(feature.c_str());
    if(fvalue == value)return; //already set!
    feature_ptr_->SetValue(value.c_str());
    std::string val;
    feature_ptr_->GetValue(val);
    log_->Info("{} {} = {}", name_, feature, val);

}

void VmbCamera::SetFeature(std::string feature, VmbInt64_t value){
    VmbInt64_t fvalue = GetFeatureInt(feature.c_str());
    if(fvalue == value || fvalue == -1)return; //already set!
    feature_ptr_->SetValue(value);
    feature_ptr_->GetValue(value);
    log_->Info("{} {} = {}", name_, feature, value);

}

void VmbCamera::SetFeature(std::string feature, double value){
    double fvalue = GetFeatureDouble(feature.c_str());
    if(fvalue == value || fvalue == -1)return; //already set!
    feature_ptr_->SetValue(value);
    feature_ptr_->GetValue(value);
    log_->Info("{} {} = {}", name_, feature, value);

}

std::string VmbCamera::GetFeatureString(std::string fname){
    std::string fval;
    auto val = camera_->GetFeatureByName(fname.c_str(), feature_ptr_);
        if(val != VmbErrorSuccess){
        log_->Info("VmbCamera::GetFeatureString failed with code {}", GetVimbaCode(val));
        return "no data";
    }
    feature_ptr_->GetValue(fval);
    log_->Info("VmbCamera::GetFeature {} {} = {}", name_, fname, fval);
    return fval;
}

VmbInt64_t VmbCamera::GetFeatureInt(std::string fname){
    VmbInt64_t fval;
    auto val = camera_->GetFeatureByName(fname.c_str(), feature_ptr_);
    if(val != VmbErrorSuccess){
        log_->Info("VmbCamera::GetFeatureInt failed with code {}", GetVimbaCode(val));
        return -1;
    }
    feature_ptr_->GetValue(fval);
    log_->Info("VmbCamera::GetFeature {} {} = {}", name_, fname, fval);
    return fval;
}

double VmbCamera::GetFeatureDouble(std::string fname){
    double fval;
    auto val = camera_->GetFeatureByName(fname.c_str(), feature_ptr_);
    if(val != VmbErrorSuccess){
        log_->Info("VmbCamera::GetFeatureDouble failed with code {}", GetVimbaCode(val));
        return -1;
    }
    feature_ptr_->GetValue(fval);
    log_->Info("VmbCamera::GetFeature {} {} = {}", name_, fname, fval);
    return fval;
}

void VmbCamera::AdjustPacketSize(){
    StreamPtrVector streams;
     
    // VmbFeatureEnumAsInt
    auto err = camera_->GetStreams(streams);

    if (err != VmbErrorSuccess || streams.empty()) {
        log_->Error("Could not get stream on {} [{}]", name_, GetVimbaCode(err));
        return;
    }

    FeaturePtr feature;
    err = streams[0]->GetFeatureByName("GVSPAdjustPacketSize", feature);

    if (err != VmbErrorSuccess){
        log_->Error("Could not get stream on {} [{}]", name_, GetVimbaCode(err));
        return;
    }

    err = feature->RunCommand();
    if (err == VmbErrorSuccess){
        auto done = false;
        while(!done){
            if (feature->IsCommandDone(done) != VmbErrorSuccess) break;                
        }
    }
    else log_->Error("Error while executing GVSPAdjustPacketSize on {}  [{}]", name_, GetVimbaCode(err));        
    
    streams[0]->GetFeatureByName("GVSPPacketSize", feature);
    VmbInt64_t val;
    feature->GetValue(val);
    log_->Info("VmbCamera::AdjustPacketSize done on {} with size {} [{}]", name_, val, GetVimbaCode(err));
}