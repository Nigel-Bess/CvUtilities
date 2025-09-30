#include <Fulfil.Dispense/tray/tray_camera_calibration.h>
#include <Fulfil.DepthCam/mocks/mock_session.h>

using fulfil::depthcam::Session;
using fulfil::depthcam::mocks::MockSession;

// Mock device manager
class MockDeviceManager : public fulfil::depthcam::DeviceManager{
public:
    MockDeviceManager(const std::string& images_directory) {
        // Create a mock session with provided directory
        std::shared_ptr<std::string> images_directory_path =
            std::make_shared<std::string>(images_directory);
        auto session = std::make_shared<MockSession>(images_directory_path, "MOCK_CAMERA_001");
        sessions_.push_back(session);
    }
    
    // Constructor that matches the original DeviceManager interface
    MockDeviceManager(const std::string& images_directory, std::vector<std::string> expected_devices, bool frozen) {
        std::shared_ptr<std::string> images_directory_path = 
            std::make_shared<std::string>(images_directory);
        // Create mock sessions for each requested serial
        for (const auto& device : expected_devices) {
            auto session = std::make_shared<MockSession>(images_directory_path, device);
            sessions_.push_back(session);
        }
        // If no serials provided, create a default one
        if (sessions_.empty()) {
            auto session = std::make_shared<MockSession>(images_directory_path, "MOCK_CAMERA_001");
            sessions_.push_back(session);
        }
    }

    std::shared_ptr<std::vector<std::shared_ptr<Session>>> get_connected_sessions() override {
        auto sessions = std::make_shared<std::vector<std::shared_ptr<Session>>>();
        sessions->insert(sessions->end(), sessions_.begin(), sessions_.end());
        return sessions;
    }

    std::shared_ptr<Session> session_by_serial_number(const std::string& serial) override {
        for (auto& session : sessions_) {
            if (*session->get_serial_number() == serial) {
                return session;
            }
        }
        return nullptr;
    }

private:
    std::vector<std::shared_ptr<MockSession>> sessions_;
};

// Function to parse command line arguments
std::string parseImageDirectory(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Error: Images directory path is mandatory!" << std::endl;
        std::cerr << "Usage: " << argv[0] << " <images_directory_path>" << std::endl;
        std::cerr << "Example: " << argv[0] << " /path/to/images/directory/" << std::endl;
        exit(1);
    }
    
    std::string imagesDir = argv[1];
    std::cout << "Using images directory: " << imagesDir << std::endl;
    
    // Add trailing slash if not present
    if (!imagesDir.empty() && imagesDir.back() != '/' && imagesDir.back() != '\\') {
        imagesDir += "/";
    }
    
    return imagesDir;
}


int main(int argc, char** argv)
{
    std::cout << "STARTING TEST SCRIPT" << std::endl;

    // Parse command line arguments
    std::string imagesDirectory = parseImageDirectory(argc, argv);

    Logger::Instance()->SetConsoleLogLevel(Logger::Level::Trace);
    Logger::Instance()->SetFileLogLevel(Logger::Level::Debug);

    std::cout << "Performing test for the hover position !\n";

    std::shared_ptr<fulfil::depthcam::DeviceManager> deviceManager = std::make_shared<MockDeviceManager>(imagesDirectory);

    fulfil::dispense::tray::TrayCameraCalibration calibration{deviceManager};
    
    try {
        auto result = calibration.tryCameraCalibrationCycle();
        
        if (result.return_code == fulfil::utils::commands::dc_api_error_codes::DcApiErrorCode::Success) {
            std::cout << "Calibration completed successfully!" << std::endl;
        } else {
            std::cout << "Calibration failed with error code: " << static_cast<int>(result.return_code) << std::endl;
            std::cout << "Error description: " << result.error_description << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception caught in main: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
