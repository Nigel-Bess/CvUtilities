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

class DropZoneSearcherTest : public ::testing::Test
{
protected:

    const std::string test_data_directory = std::string(TEST_DATA_DIR  "/pre_drop_image_side_dispense_request");
    const std::string pre_drop_request_example_path = std::string(test_data_directory +  "/preDropImageSideDispenseRequestExample.json");
    PreDropImageSideDispenseRequest request;

    void SetUp() override
    {
        request = read_in_json(pre_drop_request_example_path)->get<PreDropImageSideDispenseRequest>();

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

ArucoTagMatch find_tag_pair(std::vector<ArucoTagMatch> arucoMatches, int id) {
  auto iterator = std::find_if(arucoMatches.begin(), arucoMatches.end(),
                         [id](const auto& m) { return m.TagDefinitionAtIdenityTransform.Id == id; });
  if (iterator == arucoMatches.end()) throw std::out_of_range("ArucoTagMatch " + std::to_string(id) + " not found");
  return *iterator;
}

static void expect_vec3_eq(const Eigen::Vector3d& actual, const Eigen::Vector3d& expected)
{
    EXPECT_DOUBLE_EQ(actual.x(), expected.x());
    EXPECT_DOUBLE_EQ(actual.y(), expected.y());
    EXPECT_DOUBLE_EQ(actual.z(), expected.z());
}

TEST_F(DropZoneSearcherTest, MatchArucos)
{
    // The JSON file contains the following tags:
    //  - Tag 4 at position (30.5, 306.5, 1.0)
    //  - Tag 5 at position (108.0, 306.5, 1.0)
    //  - Tag 2 at position (381.0, 83.0, 2.0)
    //  - Tag 3 at position (381.0, 152.0, 2.0)

    // Things that make this test case interesting:
    // - Aruco tag 5 is expected but we never detected it (failed to detect marker)
    // - Aruco tag 2 appears twice  (we detected a marker on another bot)
    // - Aruco tag 3 is quite far from where we expected (we detected a marker on another bot, and failed to detect the marker on the correct bot)
    // - None of the Aruco tags are precisely where we expect them to be (measurement noise)

    // Expected behavior:
    // We find a match for Aruco tags 4 and 2. The extraneous tag 2 is discarded. Tag 3 is discarded for being too far away

    std::vector<ArucoTag> tags{
        {Eigen::Vector3d{ 35.4, 320.5, 460.0 }, 4}, // valid        
        {Eigen::Vector3d{7382.0,  833.1, 12235.9 }, 2}, // extraneous
        {Eigen::Vector3d{392.3,  71.1, 477.0 }, 2}, // valid
        {Eigen::Vector3d{481.0, 252.0, 472.0 }, 3} // more than 100mm away - erroneous detection
        // tag 5 missing altogether
    };

    RigidTransformation pose {
        Eigen::Matrix3d::Identity(),
        Eigen::Vector3d{0.0, 0.0, 470.0}
    };

   std::vector<ArucoTagMatch> arucoMatches = DropZoneSearcher::match_aruco_tags(request.ArucoTags, tags, pose, 100);

   // we expect to find 2 matches (4 and 2)
   EXPECT_EQ(arucoMatches.size(),2u);
   ArucoTagMatch tag4 = find_tag_pair(arucoMatches,4);
   ArucoTagMatch tag2 = find_tag_pair(arucoMatches,2);

   expect_vec3_eq(tag4.MeasuredPosition,tags[0].Position);
   expect_vec3_eq(tag2.MeasuredPosition,tags[2].Position);
   expect_vec3_eq(tag4.TagDefinitionAtIdenityTransform.Position,Eigen::Vector3d{ 30.5, 306.5, 1.0 });
   expect_vec3_eq(tag2.TagDefinitionAtIdenityTransform.Position,Eigen::Vector3d{ 381, 83.0, 2.0 });
}


TEST_F(DropZoneSearcherTest, ComputeCentroid)
{
        const std::vector<Eigen::Vector3d> points{
        { 1.5,  -2.25,  3.75 },
        { -4.5,  6.25, -1.75 },
        { 2.0,  -1.0,  0.5  }
    };

    const Eigen::Vector3d expected{
        -0.3333333333333333,
         1.0,
         0.8333333333333334
    };

    const auto centroid = DropZoneSearcher::compute_centroid(points);

    EXPECT_TRUE(centroid.isApprox(expected));
}



// main to run the tests above
int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
