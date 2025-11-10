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

using fulfil::depthcam::mocks::MockSession;
using fulfil::depthcam::aruco::MarkerDetectorContainer;
using fulfil::dispense::drop::DropZoneSearcher;
using ff_mongo_cpp::MongoConnection;
using fulfil::configuration::lfb::LfbVisionConfiguration;

class SideDispenseTest : public ::testing::Test
{
protected:

    //PreSideDispense
    std::shared_ptr<MockSession> pre_mock_session;
    std::shared_ptr<MarkerDetectorContainer> pre_marker_detector_container;

    //PostSideDispense
    std::shared_ptr<MockSession> post_mock_session;
    std::shared_ptr<MarkerDetectorContainer> post_marker_detector_container;

    const std::string root_test_dir = std::string(TEST_DATA_DIR "/side_dispense/68d464255f1d42c93219e469");

    std::shared_ptr<std::string> pre_test_data_directory = std::make_shared<std::string>(root_test_dir + "/Pre_Side_Dispense");
    std::shared_ptr<std::string> post_test_data_directory = std::make_shared<std::string>(root_test_dir + "/Post_Side_Dispense");

    std::shared_ptr<nlohmann::json> pre_bag_state_json;
    std::shared_ptr<nlohmann::json> post_bag_state_json;

    std::shared_ptr<LfbVisionConfiguration> pre_lfb_vision_config;
    std::shared_ptr<LfbVisionConfiguration> post_lfb_vision_config;


    void SetUp() override
    {
        pre_bag_state_json = read_in_json(*pre_test_data_directory + std::string("/2025_09_24_H21_M35_S33"), "bag_state.json");
        post_bag_state_json = read_in_json(*post_test_data_directory + std::string("/2025_09_24_H21_M35_S53"), "bag_state.json");

        pre_lfb_vision_config = std::make_shared<LfbVisionConfiguration>(
            std::make_shared<nlohmann::json>((*pre_bag_state_json)["LfbConfig"])
        );

        post_lfb_vision_config = std::make_shared<LfbVisionConfiguration>(
            std::make_shared<nlohmann::json>((*post_bag_state_json)["LfbConfig"])
        );

        pre_mock_session = std::make_shared<MockSession>(pre_test_data_directory, "MOCK_CAMERA_001");
        pre_marker_detector_container = createMarkerDetectorContainer(pre_mock_session, pre_lfb_vision_config);

        post_mock_session = std::make_shared<MockSession>(post_test_data_directory, "MOCK_CAMERA_002");
        post_marker_detector_container = createMarkerDetectorContainer(post_mock_session, post_lfb_vision_config);

    }

    void TearDown() override
    {
    }

    /**
     * Reads the JSON found at the given location into a json obj
     */
    std::shared_ptr<nlohmann::json> read_in_json(std::string directory_path,
                                                        std::string json_file_name) {
        std::string file_path = make_media::paths::join_as_path(directory_path, json_file_name);
        Logger::Instance()->Debug("JSON File location is: {}", file_path);
        std::ifstream ifs(file_path);
        std::shared_ptr<nlohmann::json> json_contents = std::make_shared<nlohmann::json>(nlohmann::json::parse(ifs));
        return json_contents;
    }

    std::shared_ptr<MarkerDetectorContainer> createMarkerDetectorContainer(
        std::shared_ptr<MockSession> session,
        std::shared_ptr<LfbVisionConfiguration> lfb_vision_config
    )
    {
        const int num_markers = lfb_vision_config->num_markers;

        std::shared_ptr<Eigen::Matrix3Xd> marker_coordinates = std::make_shared<Eigen::Matrix3Xd>(3, 8);
        for (int i = 0; i < num_markers; i++) (*marker_coordinates)(0, i) = lfb_vision_config->marker_coordinates_x[i];
        for (int i = 0; i < num_markers; i++) (*marker_coordinates)(1, i) = lfb_vision_config->marker_coordinates_y[i];
        for (int i = 0; i < num_markers; i++) (*marker_coordinates)(2, i) = lfb_vision_config->marker_coordinates_z[i];

        std::shared_ptr<fulfil::depthcam::aruco::MarkerDetector> marker_detector = std::make_shared<fulfil::depthcam::aruco::MarkerDetector>(num_markers, 4);

        std::shared_ptr<MarkerDetectorContainer> container = std::make_shared<MarkerDetectorContainer>(
            marker_detector,
            session,
            true, // validate markers
            lfb_vision_config->is_side_dispense, // is side dispense
            true, // extend region over markers
            lfb_vision_config->container_width, // container width
            lfb_vision_config->container_length, // container length
            lfb_vision_config->LFB_width, // lfb width
            lfb_vision_config->LFB_length, // lfb length
            MarkerDetectorContainer::centers_and_sides(num_markers),
            marker_coordinates,
            num_markers,
            lfb_vision_config->marker_depth, // marker depth
            lfb_vision_config->marker_depth_tolerance, // marker depth tolerance
            lfb_vision_config->min_marker_count_for_validation, // min marker count for validation
            lfb_vision_config->region_max_x, // region max x
            lfb_vision_config->region_min_x, // region min x
            lfb_vision_config->region_max_y, // region max y
            lfb_vision_config->region_min_y   // region min y
        );

        return container;
    }
};

TEST_F(SideDispenseTest, VariableCreation)
{
    ASSERT_NE(pre_mock_session, nullptr);
    ASSERT_NE(pre_marker_detector_container, nullptr);

    ASSERT_NE(post_mock_session, nullptr);
    ASSERT_NE(post_marker_detector_container, nullptr);

    ASSERT_NE(pre_bag_state_json, nullptr);
    ASSERT_NE(post_bag_state_json, nullptr);

    ASSERT_NE(pre_lfb_vision_config, nullptr);
    ASSERT_NE(post_lfb_vision_config, nullptr);

}

TEST_F(SideDispenseTest, PreSideDispenseLFBConfigTest)
{
    EXPECT_TRUE(pre_lfb_vision_config->is_side_dispense);
    EXPECT_FLOAT_EQ(pre_lfb_vision_config->container_width, 0.345);
    EXPECT_FLOAT_EQ(pre_lfb_vision_config->container_length, 0.24);
    EXPECT_FLOAT_EQ(pre_lfb_vision_config->LFB_width, 0.36);
    EXPECT_FLOAT_EQ(pre_lfb_vision_config->LFB_length, 0.241);
    EXPECT_EQ(pre_lfb_vision_config->num_markers, 8);
    EXPECT_FLOAT_EQ(pre_lfb_vision_config->marker_depth, 0.4);
    EXPECT_FLOAT_EQ(pre_lfb_vision_config->marker_depth_tolerance, 0.5);
    EXPECT_EQ(pre_lfb_vision_config->min_marker_count_for_validation, 3);
    EXPECT_EQ(pre_lfb_vision_config->region_max_x, 1150);
    EXPECT_EQ(pre_lfb_vision_config->region_min_x, 150);
    EXPECT_EQ(pre_lfb_vision_config->region_max_y, 700);
    EXPECT_EQ(pre_lfb_vision_config->region_min_y, 70);

    const float marker_coordinates_x[] = {-0.12544,-0.0505,-0.23425,-0.248,-0.248,0.248,0.22,0.222};
    const float marker_coordinates_y[] = {-0.1869,-0.1869,-0.015,0.044,0.125,0.125,0.0581,-0.0169};
    const float marker_coordinates_z[] = {0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};

    for (int i = 0; i < pre_lfb_vision_config->num_markers; i++)
    {
        EXPECT_FLOAT_EQ(pre_lfb_vision_config->marker_coordinates_x[i], marker_coordinates_x[i]);
        EXPECT_FLOAT_EQ(pre_lfb_vision_config->marker_coordinates_y[i], marker_coordinates_y[i]);
        EXPECT_FLOAT_EQ(pre_lfb_vision_config->marker_coordinates_z[i], marker_coordinates_z[i]);
    }
}

TEST_F(SideDispenseTest, PostSideDispenseLFBConfigTest)
{
    EXPECT_TRUE(post_lfb_vision_config->is_side_dispense);
    EXPECT_FLOAT_EQ(post_lfb_vision_config->container_width, 0.345);
    EXPECT_FLOAT_EQ(post_lfb_vision_config->container_length, 0.24);
    EXPECT_FLOAT_EQ(post_lfb_vision_config->LFB_width, 0.36);
    EXPECT_FLOAT_EQ(post_lfb_vision_config->LFB_length, 0.241);
    EXPECT_EQ(post_lfb_vision_config->num_markers, 8);
    EXPECT_FLOAT_EQ(post_lfb_vision_config->marker_depth, 0.4);
    EXPECT_FLOAT_EQ(post_lfb_vision_config->marker_depth_tolerance, 0.5);
    EXPECT_EQ(post_lfb_vision_config->min_marker_count_for_validation, 3);
    EXPECT_EQ(post_lfb_vision_config->region_max_x, 1150);
    EXPECT_EQ(post_lfb_vision_config->region_min_x, 150);
    EXPECT_EQ(post_lfb_vision_config->region_max_y, 700);
    EXPECT_EQ(post_lfb_vision_config->region_min_y, 70);

    const float marker_coordinates_x[] = {-0.12544,-0.0505,-0.23425,-0.248,-0.248,0.248,0.22,0.222};
    const float marker_coordinates_y[] = {-0.1869,-0.1869,-0.015,0.044,0.125,0.125,0.0581,-0.0169};
    const float marker_coordinates_z[] = {0.0,0.0,0.0,0.0,0.0,0.0,0.0,0.0};

    for (int i = 0; i < post_lfb_vision_config->num_markers; i++)
    {
        EXPECT_FLOAT_EQ(post_lfb_vision_config->marker_coordinates_x[i], marker_coordinates_x[i]);
        EXPECT_FLOAT_EQ(post_lfb_vision_config->marker_coordinates_y[i], marker_coordinates_y[i]);
        EXPECT_FLOAT_EQ(post_lfb_vision_config->marker_coordinates_z[i], marker_coordinates_z[i]);
    }
}

TEST_F(SideDispenseTest, PreSideDispenseOccupancyMapGeneration)
{
    int occ_map_width = 30;
    int occ_map_height = 30;
    
    bool is_empty = false;

    std::shared_ptr<fulfil::depthcam::pointcloud::LocalPointCloud> point_cloud = pre_marker_detector_container->get_point_cloud(false, __FUNCTION__)->as_local_cloud();
    ASSERT_NE(point_cloud, nullptr);

    int num_point_cloud_points = point_cloud->get_data()->cols();

    EXPECT_EQ(num_point_cloud_points, 1570);

    float lfb_bag_width = 0.365;
    float lfb_bag_length = 0.25;

    std::shared_ptr<std::vector<std::shared_ptr<std::vector<float>>>> occupancy_map_float = DropZoneSearcher::generate_occupancy_map(
        point_cloud,
        occ_map_width,
        occ_map_height,
        lfb_bag_width,
        lfb_bag_length,
        is_empty
    );

    ASSERT_NE(occupancy_map_float, nullptr);

    auto occupancy_map = fulfil::utils::convert_map_to_millimeters(occupancy_map_float);

    std::vector<std::vector<int>> ground_truth_occ_map =
        {
            {424,374,0,183,183,0,0,257,282,292,316,333,342,353,351,348,335,332,268,264,261,388,276,0,294,297,314,312,392,391},
            {426,190,182,181,0,235,249,264,287,295,312,333,351,353,352,350,336,294,270,264,262,256,252,244,242,299,309,310,313,311},
            {391,183,183,182,0,0,256,264,287,298,309,333,347,354,352,350,338,0,270,264,259,255,249,246,272,300,308,309,311,304},
            {386,196,183,183,0,0,260,262,288,303,311,333,344,353,354,345,339,0,270,264,258,256,250,246,269,300,311,309,311,306},
            {387,181,181,183,0,0,262,270,300,305,316,336,345,352,355,346,340,270,268,265,259,256,248,245,255,305,311,315,314,311},
            {385,182,179,181,177,255,259,270,297,318,327,339,349,355,355,347,335,284,267,265,258,255,247,244,293,305,312,314,313,310},
            {395,181,180,182,181,255,258,284,301,320,326,342,351,356,355,346,339,268,267,265,258,254,247,245,293,305,313,314,311,308},
            {386,182,180,182,183,0,260,284,293,315,334,344,354,356,354,345,337,279,266,261,257,254,247,243,295,305,310,314,311,308},
            {383,181,180,181,0,249,254,281,290,314,331,351,359,357,355,345,336,269,266,261,259,251,247,243,295,305,312,313,313,302},
            {385,181,179,182,0,247,249,279,288,308,328,349,358,354,351,344,335,269,267,260,258,249,247,240,295,304,311,314,313,304},
            {384,-28,179,0,0,245,266,268,288,304,320,333,358,353,350,342,332,269,266,260,258,249,245,241,294,303,314,314,311,301},
            {382,387,-28,-28,-28,0,262,262,282,292,152,201,340,335,271,282,329,268,265,260,256,249,246,243,288,306,312,314,310,298},
            {384,387,-29,-28,-28,0,0,145,148,150,151,199,206,240,272,276,276,269,265,260,256,249,246,239,282,287,301,298,305,293},
            {387,388,-35,-30,0,0,0,146,148,150,151,0,205,238,271,275,275,268,265,259,253,249,244,238,280,289,298,295,298,291},
            {386,387,0,0,-28,0,0,147,148,148,150,194,200,236,269,274,273,267,260,259,252,248,240,237,264,294,302,299,296,291},
            {362,-31,-32,-31,-28,0,0,147,148,148,152,154,211,234,263,274,270,267,261,257,250,247,240,237,265,296,304,302,297,290},
            {364,-28,-28,-28,-27,0,0,144,146,148,149,0,204,253,271,275,269,266,262,257,250,246,240,238,258,292,292,298,297,290},
            {364,-26,-27,-27,0,0,0,144,145,147,0,150,153,154,155,0,0,0,0,0,251,247,241,238,260,287,290,291,290,285},
            {372,-28,0,0,-27,-27,0,0,144,145,148,149,154,155,158,0,72,41,-13,-23,0,0,0,0,271,280,290,290,286,284},
            {375,0,-28,-28,-27,-27,0,0,144,0,147,150,153,154,157,0,-30,-28,0,0,-20,-29,-30,-29,276,282,293,293,291,284},
            {0,375,-31,-31,-29,-27,0,0,144,145,149,151,153,154,160,0,0,0,-26,-23,-26,-26,-24,277,283,292,297,297,294,0},
            {361,-36,-34,-31,0,0,0,0,148,145,147,151,153,155,159,0,-29,-28,-25,-23,-25,-26,-25,0,-26,-28,298,298,0,0},
            {363,-29,0,0,-28,0,0,0,148,146,146,152,152,153,158,0,-28,-29,-28,-26,-26,0,0,-25,-23,-24,-26,0,0,0},
            {0,0,-30,-28,-28,0,0,0,143,143,143,151,148,147,155,0,-29,-29,0,0,0,-28,-28,-26,-23,-23,-28,0,0,0},
            {-29,-30,-32,-29,-29,0,0,0,138,139,138,0,0,0,0,0,0,0,-29,-29,-29,-29,-25,-29,-28,0,0,-31,-28,-28},
            {0,88,0,30,31,32,0,0,0,0,0,0,0,0,0,0,-30,-30,-28,-28,-28,-30,0,0,0,-33,-34,-30,-29,-28},
            {92,394,395,139,248,32,0,0,0,0,82,0,0,0,0,0,74,68,60,-28,-28,0,-28,-31,-31,-30,-32,-28,-26,-25},
            {0,400,400,391,384,390,370,378,333,336,91,310,149,161,163,116,139,142,60,0,-30,119,-29,77,69,65,64,-28,0,0},
            {0,410,410,393,389,390,390,390,390,389,386,388,386,368,372,389,383,360,367,314,167,265,296,304,151,64,0,85,400,-21},
            {424,425,389,181,183,256,266,274,303,302,322,342,354,352,351,341,331,336,267,374,391,257,391,391,391,391,391,391,390,392}
        };

    ASSERT_EQ(occupancy_map->size() * (*occupancy_map)[0]->size(), ground_truth_occ_map.size() * ground_truth_occ_map[0].size());

    for (int i = 0; i < occupancy_map->size(); i++)
    {
        for (int j = 0; j < (*occupancy_map)[i]->size(); j++)
        {
            EXPECT_EQ((*(*occupancy_map)[i])[j], ground_truth_occ_map[i][j]);
        }
    }
}

TEST_F(SideDispenseTest, PostSideDispenseOccupancyMapGeneration)
{
    int occ_map_width = 5;
    int occ_map_height = 5;
    
    bool is_empty = false;

    std::shared_ptr<fulfil::depthcam::pointcloud::LocalPointCloud> point_cloud = post_marker_detector_container->get_point_cloud(false, __FUNCTION__)->as_local_cloud();
    ASSERT_NE(point_cloud, nullptr);

    int num_point_cloud_points = point_cloud->get_data()->cols();

    EXPECT_EQ(num_point_cloud_points, 1571);

    float lfb_bag_width = 0.365;
    float lfb_bag_length = 0.25;

    std::shared_ptr<std::vector<std::shared_ptr<std::vector<float>>>> occupancy_map_float = DropZoneSearcher::generate_occupancy_map(
        point_cloud,
        occ_map_width,
        occ_map_height,
        lfb_bag_width,
        lfb_bag_length,
        is_empty
    );

    ASSERT_NE(occupancy_map_float, nullptr);

    auto occupancy_map = fulfil::utils::convert_map_to_millimeters(occupancy_map_float);

    std::vector<std::vector<int>> ground_truth_occ_map =
        {
            {186, 265, 355, 328, 302},
            {-29, 258, 339, 334, 306},
            {390, 144, 157, 331, 290},
            {390, 141, 155, -23, 288},
            {389, 345, 388, 392, 399}
        };
    
    ASSERT_EQ(occupancy_map->size() * (*occupancy_map)[0]->size(), ground_truth_occ_map.size() * ground_truth_occ_map[0].size());

    for (int i = 0; i < occupancy_map->size(); i++)
    {
        for (int j = 0; j < (*occupancy_map)[i]->size(); j++)
        {
            EXPECT_EQ((*(*occupancy_map)[i])[j], ground_truth_occ_map[i][j]);
        }
    }
}



// main to run the tests above
int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
