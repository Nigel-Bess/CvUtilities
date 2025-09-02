#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include "libobsensor/ObSensor.hpp"

using namespace std;

int main()
{
    ob::Context ctx;
    std::shared_ptr<ob::DeviceList> devices = ctx.queryDeviceList();
    for (int i = 0; i < 2; i++) {
        auto device = devices->getDevice(i)->getDeviceInfo();
        // Log device info
        cout << "  SN: " << device->serialNumber() << endl;
        cout << "  name: " << device->name() << endl;
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(300000));
    return 0;
}

/*int main()
{
    ob::Context ctx;
    std::shared_ptr<ob::DeviceList> devices = ctx.queryDeviceList();
    auto device = devices->getDeviceBySN("AE4M73D0040");
    auto pipe = std::make_shared<ob::Pipeline>(device);
    // Configure which streams to enable or disable for the Pipeline by creating a Config
    std::shared_ptr<ob::Config> config = std::make_shared<ob::Config>();
    config->enableVideoStream(OB_STREAM_COLOR);
    // Start the pipeline with config
    pipe->start(config);
    
    auto frameSet = pipe->waitForFrames(500);
    auto colorFrame = frameSet->colorFrame();
    auto index = colorFrame->index(); 
    if(index % 30 == 0) {
        std::cout << "*************************** Color Frame #" << index << " Metadata List ********************************" << std::endl;
        for(int metaDataType = 0; metaDataType < OB_FRAME_METADATA_TYPE_COUNT; metaDataType++) {
            // Check if it is supported metaDataType for current frame
            if(colorFrame->hasMetadata((OBFrameMetadataType)metaDataType)) {
                // Get the value of the metadata
                //std::cout << metaDataTypes[metaDataType] << ": " << colorFrame->getMetadataValue((OBFrameMetadataType)metaDataType) << std::endl;
            }
            else {
                //std::cout << metaDataTypes[metaDataType] << ": " << "unsupported" << std::endl;
            }
        }
        std::cout << "********************************************************************************" << std::endl << std::endl;

    }
    pipe->stop();
}*/
