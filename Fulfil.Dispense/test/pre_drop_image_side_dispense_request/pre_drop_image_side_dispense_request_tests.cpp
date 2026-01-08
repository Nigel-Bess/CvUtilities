#include <gtest/gtest.h>
#include <Fulfil.DepthCam/mocks/mock_session.h>
#include <Fulfil.DepthCam/aruco/marker_detector_container.h>
#include <Fulfil.DepthCam/point_cloud/local_point_cloud.h>
#include <Fulfil.Dispense/drop/drop_zone_searcher.h>
#include <Fulfil.CPPUtils/inih/INIReader.h>
#include <Fulfil.CPPUtils/conversions.h>
#include <Fulfil.Dispense/visualization/make_media.h>
#include <FulfilMongoCpp/mongo_connection.h>
#include <Fulfil.Dispense/recipes/lfb_vision_configuration.h>
#include <Fulfil.Dispense/commands/pre_side_dispense/aruco_tag.h>
#include <Fulfil.Dispense/commands/pre_side_dispense/pre_drop_image_side_dispense_request.h>

using fulfil::depthcam::mocks::MockSession;
using fulfil::depthcam::aruco::MarkerDetectorContainer;
using fulfil::dispense::drop::DropZoneSearcher;
using ff_mongo_cpp::MongoConnection;
using fulfil::configuration::lfb::LfbVisionConfiguration;

class PreDropImageSideDipsenseRequestTest : public ::testing::Test
{
protected:

    const std::string test_data_directory = std::string(TEST_DATA_DIR  "/pre_drop_image_side_dispense_request");
    const std::string pre_drop_request_example_path = std::string(test_data_directory +  "/preDropImageSideDispenseRequestExample.json");
    std::shared_ptr<nlohmann::json> request_json;

    void SetUp() override
    {
        request_json = read_in_json(pre_drop_request_example_path);

    }

    void TearDown() override
    {
    }

    
    /**
     * Reads the JSON found at the given location into a json obj
     */
    std::shared_ptr<nlohmann::json> read_in_json(std::string file_path) {
        Logger::Instance()->Debug("Loading Json from: {}", file_path);
        std::ifstream ifs(file_path);
        std::shared_ptr<nlohmann::json> json_contents = std::make_shared<nlohmann::json>(nlohmann::json::parse(ifs));
        return json_contents;
    }

};

TEST_F(PreDropImageSideDipsenseRequestTest, ParseArucos)
{
    std::vector<ArucoTag> tags = request_json->at("Aruco_Tags").get<std::vector<ArucoTag>>();
    // find Aruco tag with id = 2
    std::vector<ArucoTag>::const_iterator iterator = std::find_if(tags.begin(), tags.end(),[](const ArucoTag& t) {return t.Id == 2; });
    ASSERT_NE(iterator, tags.end());

    // verify that Aruco tag with id = 2 has position = (_,83,_)
    EXPECT_EQ(iterator->Position[1],83);
}

TEST_F(PreDropImageSideDipsenseRequestTest, ParseRequest)
{
    PreDropImageSideDispenseRequest request = request_json->get<PreDropImageSideDispenseRequest>();
    std::vector<ArucoTag> tags = request.ArucoTags;

        // find Aruco tag with id = 3
    std::vector<ArucoTag>::const_iterator iterator = std::find_if(tags.begin(), tags.end(),[](const ArucoTag& t) {return t.Id == 3; });
    ASSERT_NE(iterator, tags.end());

    // verify that Aruco tag with id = 3 has position = (381,_,_)
    EXPECT_EQ(iterator->Position[0],381);

    EXPECT_EQ(request.BagCavityDimensions[2],430);
}




// main to run the tests above
int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
