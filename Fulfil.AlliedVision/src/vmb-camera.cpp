#include "vmb-camera.h"
#include "commands/bag_release/repack_error_codes.h"

using fulfil::dispense::commands::RepackErrorCodes;
using fulfil::dispense::commands::get_error_name_from_code;

// Static lock is shared across all camera instances so only 1 camera
// may actively stream frames at a time, this is to avoid bad networking
// issues observed with Vimba cameras for Repack
static std::mutex _streamingCamLock;

/// Max snapshot retries till GetBlockingImage gives up taking a valid frame
const int MAX_SNAPSHOT_RETRIES = 10;
/// Number of frames to take in multi acquire mode, larger values make GetBlockingImage slower
/// but increase the odds of getting a Complete frame, leave low since there's a parent
/// retry loop set by MAX_SNAPSHOT_RETRIES anyway
const uint32_t MULTIFRAME_COUNT = 4;

VmbCamera::VmbCamera(std::string ip, int bay, fulfil::utils::Logger* log, std::shared_ptr<GrpcService> serv): 
                camera_ip_(ip), bay_(bay), log_(log), service_(serv){
    SetName();
}

void VmbCamera::StartCamera(){
    std::thread(&VmbCamera::RunCamera, this).detach();
}

void VmbCamera::KillCamera(){
    run_ = false;
    connected_ = false;
    log_->Info("VmbCamera shutdown on {}", name_);
    if(camera_ != nullptr)
        camera_->Close();
}


//These bays have light sources right above the cameras which leads the image to be brighter
//Applying custom settings to make the images less brighter
bool VmbCamera::CameraHasBrightView(std::string name_) {
    return (name_ == "RepackBay03" or name_ == "RepackBay04" or name_ == "RepackBay06" 
        or name_ == "RepackBay07" or name_ == "RepackBay09" or name_ == "RepackBay10");
}

void VmbCamera::RunSetup(bool isInitSetup){
    log_->Info("VmbCamera RunSetup on {}", name_);
    auto code = camera_->Open(VmbAccessModeFull);
    auto first = true;
    while(code != VmbErrorSuccess){
        log_->Error("{} returned {} when trying to open camera", name_, GetVimbaCode(code));
        if(first)
            AddCameraStatus(DepthCameras::DcCameraStatusCodes::CAMERA_STATUS_RECOVERABLE_EXCEPTION);
        camera_->Close();
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        code = camera_->Open(VmbAccessModeFull);
        first = false;
    }
    std::string name;
    camera_->GetName(name);
    log_->Info("{} [{}] open with FWv: {}]", name_, name, GetFeatureString("DeviceFirmwareID")); 

    if (isInitSetup) {
        // Max of 5Mb upload per second to allow other cams' to have plenty of bandwidth, this should be calculated
        // based on the network switch on neighboring camera count
        log_->Info("Setting link to 50000000");
        VmbInt64_t maxBandwidthBytes = 100000000;
        SetFeature("DeviceLinkThroughputLimitMode", "On");
        SetFeature("DeviceLinkThroughputLimit", maxBandwidthBytes);
        log_->Info("Got feature {}", GetFeatureInt("DeviceLinkThroughputLimit"));

        SetFeature("PixelFormat", "BGR8");
        SetFeature("ExposureAuto", "Once");
        SetFeature("Hue", -2.0);
        SetFeature("Saturation", 1.0);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));//wait for auto exp to kick in
        //These values may need changes in future until a config is added
        if (CameraHasBrightView(name_)) {
            SetFeature("ExposureTime", 15000.0); //decresing the exposure time to reduce the light falling on the lens
            SetFeature("Gamma", 0.5); //brightness factor
        }
        else {
            SetFeature("ExposureTime", 19985.98);
            SetFeature("Gamma", 0.6);
        }
        AdjustPacketSize();
    }
    connected_ = true;
    AddCameraStatus(DepthCameras::DcCameraStatusCodes::CAMERA_STATUS_CONNECTED);
    if (isInitSetup) {
        GetImageBlocking();
        SaveLastImage(name_);
    }
}
//sudo ifconfig enp65s0f0 mtu 9000
//sudo ifconfig enp65s0f1 mtu 9000
void VmbCamera::RunCamera(){
    bool isInitConnection = true;
    while(run_){
        if(!connected_) {
            std::lock_guard<std::mutex> lock(_lifecycleLock);{
                RunSetup(isInitConnection);
                isInitConnection = false;
            }
        }
        else{
            std::this_thread::sleep_for(std::chrono::milliseconds(5000));
            std::string name;
            VmbErrorType code;
            code = camera_->GetName(name);
            // std::cout << name << std::endl;
            if(code != VmbErrorSuccess){
                log_->Error("{} returned {} when trying to poll camera, reconnecting", name_, GetVimbaCode(code));
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
        std::string img_name = path + ".jpeg";
        if(last_mat_.size().empty()){
            log_->Error("Cannot save emtpy image to {}", img_name);
            return;
        }
        cv::imwrite(img_name, last_mat_);
        log_->Info("{} saved successfully!!!", img_name);// img_name << " saved successfully!" << std::endl;

    }
    catch(const std::exception &ex){
        log_->Error("VmbManager::SaveLastImage caught error: {}", ex.what());
    }
    catch(...){
        log_->Error("VmbManager::SaveLastImage hit error in catch(...)");
    }
}

std::shared_ptr<cv::Mat> VmbCamera::GetImageBlocking(){
    VmbUint32_t height;
    VmbUint32_t width;
    auto empty = std::make_shared<cv::Mat>();
    for (int i = 0; i < MAX_SNAPSHOT_RETRIES && !connected_; i++){
        log_->Warn("GetImageBlocking stalling for camera reconnection, {} is disconnected, retry attempt #{}", name_, i);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
    if (!connected_) {
        log_->Error("GetImageBlocking returning empty because {} is disconnected", name_);
        return empty;
    }
    auto err = VmbErrorCustom;

    if (CameraHasBrightView(name_)) {
        SetFeature("ExposureTime", 15000.0);
        SetFeature("Gamma", 0.5);
    }
    else {
        SetFeature("ExposureTime", 19985.98);
        SetFeature("Gamma", 0.6);
    }
    SetFeature("Hue", -2.0);
    SetFeature("Saturation", 1.0);

    // TODO: Determine if the current cam uplink throttling completely replaces the need
    // to enable only 1 active camera at a time.
    //std::lock_guard<std::mutex> lock(_streamingCamLock);
    std::lock_guard<std::mutex> lock(_lifecycleLock);
    {
        VmbUint32_t payloadSize;
        err = camera_->GetPayloadSize( payloadSize );
        VmbFrameStatusType frameStatus;
        log_->Info("reset frame {}", name_);
        frame_ptr_.reset(new Frame(payloadSize));
        frame_ptrs_ = VmbCPP::FramePtrVector(MULTIFRAME_COUNT); 
        log_->Info("Streaming for first complete frame {}", name_);

        // Look through all images taken for hopefully a frame that's complete
        bool frameFound = false;
        for (int aquireCount=0; aquireCount < MAX_SNAPSHOT_RETRIES && !frameFound; aquireCount++) {
            for (int f = 0; f < MULTIFRAME_COUNT; f++) {
                frame_ptrs_[f].reset(new Frame(payloadSize));
            }
            log_->Info("AquiredMultiple start {}", name_);
            err = camera_->AcquireMultipleImages(frame_ptrs_, 20000);
            log_->Info("AquiredMultiple end {}", name_);
            if (err != VmbErrorSuccess) {
                camera_error_code = RepackErrorCodes::VimbaCameraError;
                camera_error_description = "{} could not get frame with code {}", name_, GetVimbaCode(err);
                log_->Error(camera_error_description);
            }
            // Better to get the LAST good frame since it's more likely to have more accurate interlacing
            int firstGood = 999999;
            int lastGood = -1;
            for (int f = MULTIFRAME_COUNT-1; f >= 0; f--) {
                frame_ptrs_[f]->GetReceiveStatus(frameStatus);
                if (frameStatus == 0) {
                    frameFound = true;
                    firstGood = f < firstGood ? f : firstGood;
                    lastGood = f > lastGood ? f : lastGood;
                } else if (f == 0) {
                    log_->Info("No good frames in {} tries (cam {}), resetting aquire mode again...", (aquireCount+1)*MULTIFRAME_COUNT, name_);
                }
            }
            if (frameFound) {
                log_->Info("First good frame at {}, using last good frame at {} on {}", 
                    aquireCount*MULTIFRAME_COUNT + firstGood,
                    aquireCount*MULTIFRAME_COUNT + lastGood,
                    name_);   
                frame_ptr_ = frame_ptrs_[lastGood];
                break;
            }
        }
        if(!frameFound){
            log_->Error("No complete frame found for {}", name_);
            return empty;
        }
    }
    if(err != VmbErrorSuccess){
        camera_error_code = RepackErrorCodes::VimbaCameraError;
        camera_error_description = "{} could not get frame with code {}", name_, GetVimbaCode(err);
        log_->Error(camera_error_description);
        return empty;
    }
    try{
        auto val = frame_ptr_->GetImage(img_buffer_);
        frame_ptr_->GetHeight(height);
        frame_ptr_->GetWidth(width);
        // std::cout << "GOT FRAME " << height << " x " << width  << std::endl;
        last_mat_ = cv::Mat(height, width, CV_8UC3, img_buffer_);
        log_->Debug("Got mat from image with height {} and width {}", (int)height, (int)width);
        last_image_ = std::make_shared<cv::Mat>(last_mat_);
        return last_image_;
    }
    catch(const std::exception &ex){
        if (camera_error_code == RepackErrorCodes::Success) {
            camera_error_code = RepackErrorCodes::VimbaCameraError;
            camera_error_description = get_error_name_from_code((RepackErrorCodes)camera_error_code) + " -> " + "VmbManager::GetImageBlocking caught error: {}", ex.what();
        }
        log_->Error(camera_error_description);
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
    log_->Info("Previous Feature value: {} {} = {}", name_, feature, fvalue);
    if(fvalue == value)return; //already set!
    feature_ptr_->SetValue(value.c_str());
    std::string val;
    feature_ptr_->GetValue(val);
    log_->Info("Latest Feature value: {} {} = {}", name_, feature, val);
}

void VmbCamera::SetFeature(std::string feature, VmbInt64_t value){
    VmbInt64_t fvalue = GetFeatureInt(feature.c_str());
    log_->Info("Previous Feature value: {} {} = {}", name_, feature, fvalue);
    if(fvalue == value || fvalue == -1)return; //already set!
    feature_ptr_->SetValue(value);
    feature_ptr_->GetValue(value);
    log_->Info("Latest Feature value: {} {} = {}", name_, feature, value);
}

void VmbCamera::SetFeature(std::string feature, double value){
    double fvalue = GetFeatureDouble(feature.c_str());
    log_->Info("Previous Feature value: {} {} = {}", name_, feature, fvalue);
    if(fvalue == value || fvalue == -1)return; //already set!
    feature_ptr_->SetValue(value);
    feature_ptr_->GetValue(value);
    log_->Info("Latest Feature value: {} {} = {}", name_, feature, value);

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