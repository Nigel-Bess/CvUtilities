#pragma once

#include <thread>
#include <iomanip>
#include <opencv2/opencv.hpp>
#include "VmbCPP/VmbCPP.h"
#include <filesystem>

#include <Fulfil.CPPUtils/comm/GrpcService.h>
#include <Fulfil.CPPUtils/logging.h>
#include "commands/bag_release/repack_error_codes.h"

using namespace VmbCPP;
using fulfil::dispense::commands::RepackErrorCodes;

class VmbCamera{
    public:
        VmbCamera(std::string ip, int bay, fulfil::utils::Logger* log, std::shared_ptr<GrpcService> serv);
        std::shared_ptr<cv::Mat> GetImageBlocking();
        std::chrono::_V2::system_clock::time_point last_exposure_reset_time = std::chrono::system_clock::now();
        void StartCamera();
        void RunAutoExposure();
        void KillCamera();
        inline bool Connected(){ return connected_; }        
        VmbCPP::CameraPtr camera_;
        std::string camera_ip_;
        std::string name_;
        std::shared_ptr<cv::Mat> last_image_;
        void SaveLastImage(std::string path);
        int camera_error_code = RepackErrorCodes::Success;
        std::string camera_error_description = "";

    private:
        VmbUchar_t* img_buffer_;
        VmbUint32_t payloadSize_;
        VmbCPP::FramePtr frame_ptr_;
        VmbCPP::FeaturePtr feature_ptr_;
        void RunCamera();
        std::string GetFeatureString(std::string fname);
        VmbInt64_t GetFeatureInt(std::string fname);
        double GetFeatureDouble(std::string fname);
        void LogPingSuccess();
        void SetFeature(std::string feature, std::string value);
        void SetFeature(std::string feature, VmbInt64_t value);
        void SetFeature(std::string feature, double value);
        void SetExposureSettings();
        void AdjustPacketSize();
        void AddCameraStatus(DepthCameras::DcCameraStatusCodes code);
        void RunSetup(bool isInitSetup);
        std::string GetMacAddress();
        volatile bool run_ = true;
        volatile bool connected_ = false;
        int bay_;
        std::recursive_mutex _lifecycleLock;
        fulfil::utils::Logger* log_;
        std::shared_ptr<GrpcService> service_;
        inline void SetName(){
            std::stringstream ss;
            ss << "RepackBay" << std::setw(2) << std::setfill('0') << bay_;
            name_ = ss.str();
        }
        bool CameraHasBrightView(std::string name_);
};


//don't worry I didn't type this out
inline std::string GetVimbaCode(VmbErrorType code){
    switch(code){
        case VmbErrorSuccess: 	return "VmbErrorSuccess";
        case VmbErrorCustom: 	return "VmbErrorCustom";
        case VmbErrorInsufficientBufferCount: 	return "VmbErrorInsufficientBufferCount";
        case VmbErrorRetriesExceeded: 	return "VmbErrorRetriesExceeded";
        case VmbErrorAmbiguous: 	return "VmbErrorAmbiguous";
        case VmbErrorTLNotFound: 	return "VmbErrorTLNotFound";
        case VmbErrorFeaturesUnavailable: 	return "VmbErrorFeaturesUnavailable";
        case VmbErrorUserCallbackException: 	return "VmbErrorUserCallbackException";
        case VmbErrorNoChunkData: 	return "VmbErrorNoChunkData";
        case VmbErrorAlready: 	return "VmbErrorAlready";
        case VmbErrorInvalidAddress: 	return "VmbErrorInvalidAddress";
        case VmbErrorNotInitialized: 	return "VmbErrorNotInitialized";
        case VmbErrorNotAvailable: 	return "VmbErrorNotAvailable";
        case VmbErrorXml: 	return "VmbErrorXml";
        case VmbErrorUnknown: 	return "VmbErrorUnknown";
        case VmbErrorInUse: 	return "VmbErrorInUse";
        case VmbErrorParsingChunkData: 	return "VmbErrorParsingChunkData";
        case VmbErrorNoData: 	return "VmbErrorNoData";
        case VmbErrorBusy: 	return "VmbErrorBusy";
        case VmbErrorUnspecified: 	return "VmbErrorUnspecified";
        case VmbErrorGenTLUnspecified: 	return "VmbErrorGenTLUnspecified";
        case VmbErrorValidValueSetNotPresent: 	return "VmbErrorValidValueSetNotPresent";
        case VmbErrorIO: 	return "VmbErrorIO";
        case VmbErrorIncomplete: 	return "VmbErrorIncomplete";
        case VmbErrorNotSupported: 	return "VmbErrorNotSupported";
        case VmbErrorNotImplemented: 	return "VmbErrorNotImplemented";
        case VmbErrorNoTL: 	return "VmbErrorNoTL";
        case VmbErrorInvalidCall: 	return "VmbErrorInvalidCall";
        case VmbErrorResources: 	return "VmbErrorResources";
        case VmbErrorOther: 	return "VmbErrorOther";
        case VmbErrorTimeout: 	return "VmbErrorTimeout";
        case VmbErrorInvalidValue: 	return "VmbErrorInvalidValue";
        case VmbErrorWrongType: 	return "VmbErrorWrongType";
        case VmbErrorMoreData: 	return "VmbErrorMoreData";
        case VmbErrorStructSize: 	return "VmbErrorStructSize";
        case VmbErrorBadParameter: 	return "VmbErrorBadParameter";
        case VmbErrorInvalidAccess: 	return "VmbErrorInvalidAccess";
        case VmbErrorDeviceNotOpen: 	return "VmbErrorDeviceNotOpen";
        case VmbErrorBadHandle: 	return "VmbErrorBadHandle";
        case VmbErrorNotFound: 	return "VmbErrorNotFound";
        case VmbErrorApiNotStarted: 	return "VmbErrorApiNotStarted";
        case VmbErrorInternalFault: 	return "VmbErrorInternalFault";
        default: return "Unknown code";
    }
}