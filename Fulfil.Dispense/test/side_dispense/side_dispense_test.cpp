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

    EXPECT_EQ(num_point_cloud_points, 7327); //number of points in the unfiltered point cloud

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
            { 383,190,184,183,183,233,246,257,274,292,316,325,342,353,351,348,335,332,268,264,261,260,248,286,294,297,314,312,392,391},
            { 426,190,182,181,0,235,249,264,287,295,312,333,351,353,352,350,336,294,270,264,262,256,252,244,242,299,309,310,313,311 },
            { 391,183,183,182,0,0,256,264,287,298,309,333,347,354,352,350,338,0,270,264,259,255,249,246,272,300,308,309,311,304 },
            { 386,196,183,183,0,0,260,262,288,303,311,333,344,353,354,345,339,0,270,264,258,256,250,246,269,300,311,309,311,306 },
            { 387,181,181,183,0,0,262,270,300,305,316,336,345,352,355,346,340,270,268,265,259,256,248,245,255,305,311,315,314,311 },
            { 385,182,179,181,177,255,259,270,297,318,327,339,349,355,355,347,335,284,267,265,258,255,247,244,293,305,312,314,313,310 },
            { 395,181,180,182,181,255,258,284,301,320,326,342,351,356,355,346,339,268,267,265,258,254,247,245,293,305,313,314,311,308 },
            { 386,182,180,182,183,0,260,284,293,315,334,344,354,356,354,345,337,279,266,261,257,254,247,243,295,305,310,314,311,308 },
            { 383,181,180,181,0,249,254,281,290,314,331,351,359,357,355,345,336,269,266,261,259,251,247,243,295,305,312,313,313,302 },
            { 385,181,179,182,0,247,249,279,288,308,328,349,358,354,351,344,335,269,267,260,258,249,247,240,295,304,311,314,313,304 },
            { 382,-28,179,0,0,245,266,268,288,304,320,333,358,353,350,342,332,269,266,260,258,249,245,241,294,303,314,314,311,301 },
            { 382,387,-28,-28,-28,0,262,262,282,292,152,201,340,335,271,282,329,268,265,260,256,249,246,243,288,306,312,314,310,298 },
            { 384,387,-29,-28,-28,0,0,145,148,150,151,199,206,240,272,276,276,269,265,260,256,249,246,239,282,287,301,298,305,293 },
            { 387,388,-35,-30,0,0,0,146,148,150,151,0,205,238,271,275,275,268,265,259,253,249,244,238,280,289,298,295,298,291 },
            { 386,387,0,0,-28,0,0,147,148,148,150,194,200,236,269,274,273,267,260,259,252,248,240,237,264,294,302,299,296,291 },
            { 362,-31,-32,-31,-28,0,0,147,148,148,152,154,211,234,263,274,270,267,261,257,250,247,240,237,265,296,304,302,297,290 },
            { 364,-28,-28,-28,-27,0,0,144,146,148,149,0,204,253,271,275,269,266,262,257,250,246,240,238,258,292,292,298,297,290 },
            { 364,-26,-27,-27,0,0,0,144,145,147,0,150,153,154,155,0,0,0,0,0,251,247,241,238,260,287,290,291,290,285 },
            { 372,-28,0,0,-27,-27,0,0,144,145,148,149,154,155,158,0,72,41,-13,-23,0,0,0,0,271,280,290,290,286,284 },
            { 375,0,-28,-28,-27,-27,0,0,144,0,147,150,153,154,157,0,-30,-28,0,0,-20,-29,-30,-29,276,282,293,293,291,284 },
            { 370,375,-31,-31,-29,-27,0,0,144,145,149,151,153,154,160,0,0,0,-26,-23,-26,-26,-24,277,283,292,297,297,294,0 },
            { 361,-36,-34,-31,0,0,0,0,148,145,147,151,153,155,159,0,-29,-28,-25,-23,-25,-26,-25,0,-26,-28,298,298,0,0 },
            { 363,-29,0,0,-28,0,0,0,148,146,146,152,152,153,158,0,-28,-29,-28,-26,-26,0,0,-25,-23,-24,-26,0,0,0 },
            { 0,0,-30,-28,-28,0,0,0,143,143,143,151,148,147,155,0,-29,-29,0,0,0,-28,-28,-26,-23,-23,-28,0,0,0 },
            { -29,-30,-32,-29,-29,0,0,0,138,139,138,0,0,0,0,0,0,0,-29,-29,-29,-29,-25,-29,-28,0,0,-31,-28,-28 },
            { 360,88,0,30,31,32,0,0,0,0,0,0,0,0,0,0,-30,-30,-28,-28,-28,-30,0,0,0,-33,-34,-30,-29,-28 },
            { 92,394,395,139,248,32,0,0,0,0,82,0,0,0,0,0,74,68,60,-28,-28,0,-28,-31,-31,-30,-32,-28,-26,-25 },
            { 365,400,400,391,384,390,370,378,333,336,91,310,149,161,163,116,139,142,60,0,-30,119,-29,77,69,65,64,-28,0,0 },
            { 0,410,410,393,389,390,390,390,390,389,386,388,386,368,372,389,383,360,367,314,167,265,296,304,151,64,0,85,400,200 },
            { 423,422,397,181,404,256,266,263,282,301,318,390,389,361,385,384,387,392,389,375,392,391,392,391,393,391,392,393,395,413 }
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

    std::shared_ptr<fulfil::depthcam::pointcloud::LocalPointCloud> point_cloud = post_marker_detector_container->get_point_cloud_post_dispense(false, __FUNCTION__)->as_local_cloud();
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

TEST_F(SideDispenseTest, SideDispenseKabschTransformation)
{
    std::vector<Eigen::Vector3d> aruco_locations_at_identity_transform = {
        Eigen::Vector3d( -153, 176.5, 1 ),
        Eigen::Vector3d( -75.5, 176.5, 1 ),
        Eigen::Vector3d( 197.5, 22, 2 )
    };
    std::vector<Eigen::Vector3d> measured_aruco_locations = {
        Eigen::Vector3d( -173, 176.5, 1 ),
        Eigen::Vector3d( -95.5, 176.5, 1 ),
        Eigen::Vector3d( 177.5, 22, 2 )
    };
    std::vector<int> inlier_indices = { 0, 1, 2 };

    DropZoneSearcher::RigidTransformation transform;
    Eigen::Matrix3d ground_truth_rotation_matrix = Eigen::Matrix3d::Identity();
    Eigen::Vector3d ground_truth_translation_vector(-20, 0, 0);
  
    transform = *DropZoneSearcher::implement_kabsch_rotation_n_translation(aruco_locations_at_identity_transform, measured_aruco_locations, inlier_indices);
    
    Eigen::Matrix3d rounded_rotation_matrix = transform.rotation_matrix.unaryExpr([](double x) {
        return (std::abs(x) < 1e-12) ? 0.0 : x;
    });

    Eigen::Vector3d rounded_translation_vector = transform.translation_vector.unaryExpr([](double x) {
        return (std::abs(x) < 1e-12) ? 0.0 : x;
    });

    EXPECT_TRUE(rounded_rotation_matrix.isApprox(ground_truth_rotation_matrix));
    EXPECT_TRUE(rounded_translation_vector.isApprox(ground_truth_translation_vector));
}

TEST_F(SideDispenseTest, SideDispenseKabschTransformationWithCenter)
{

    std::vector<Eigen::Vector3d> aruco_locations_at_identity_transform = {
         Eigen::Vector3d(-150, 170, 1),
        Eigen::Vector3d(-75, 170, 1),
        Eigen::Vector3d(190, 20, 2),
    };
    std::vector<Eigen::Vector3d> measured_aruco_locations = {
         Eigen::Vector3d(-170, 185, 1),
         Eigen::Vector3d(-95, 185, 1),
         Eigen::Vector3d(170, 35, 2),
    };
    std::vector<int> inlier_indices = { 0, 1, 2 };

    DropZoneSearcher::RigidTransformation transform;
    Eigen::Matrix3d ground_truth_rotation_matrix = Eigen::Matrix3d::Identity();
    Eigen::Vector3d ground_truth_translation_vector(-20, 15, 0);
    
    transform = *DropZoneSearcher::implement_kabsch_rotation_n_translation(aruco_locations_at_identity_transform, measured_aruco_locations, inlier_indices);
   
    Eigen::Matrix3d rounded_rotation_matrix = transform.rotation_matrix.unaryExpr([](double x) {
        return (std::abs(x) < 1e-12) ? 0.0 : x;
    });

    Eigen::Vector3d rounded_translation_vector = transform.translation_vector.unaryExpr([](double x) {
        return (std::abs(x) < 1e-12) ? 0.0 : x;
    });

    EXPECT_TRUE(rounded_rotation_matrix.isApprox(ground_truth_rotation_matrix));
    EXPECT_TRUE(rounded_translation_vector.isApprox(ground_truth_translation_vector));
}

TEST_F(SideDispenseTest, SideDispenseKabschTransformationWithTranslationAlongALL3Axis)
{

    std::vector<Eigen::Vector3d> aruco_locations_at_identity_transform = {
         Eigen::Vector3d(-150, 170, 1),
        Eigen::Vector3d(-75, 170, 1),
        Eigen::Vector3d(190, 20, 2),
    };
    std::vector<Eigen::Vector3d> measured_aruco_locations = {
         Eigen::Vector3d(-170, 185, 6),
         Eigen::Vector3d(-95, 185, 6),
         Eigen::Vector3d(170, 35, 7),
    };
    std::vector<int> inlier_indices = { 0, 1, 2 };

    DropZoneSearcher::RigidTransformation transform;
    Eigen::Matrix3d ground_truth_rotation_matrix = Eigen::Matrix3d::Identity();
    Eigen::Vector3d ground_truth_translation_vector(-20, 15, 5);
   
    transform = *DropZoneSearcher::implement_kabsch_rotation_n_translation(aruco_locations_at_identity_transform, measured_aruco_locations, inlier_indices);
   
    Eigen::Matrix3d rounded_rotation_matrix = transform.rotation_matrix.unaryExpr([](double x) {
        return (std::abs(x) < 1e-12) ? 0.0 : x;
    });

    Eigen::Vector3d rounded_translation_vector = transform.translation_vector.unaryExpr([](double x) {
        return (std::abs(x) < 1e-12) ? 0.0 : x;
    });

    EXPECT_TRUE(rounded_rotation_matrix.isApprox(ground_truth_rotation_matrix));
    EXPECT_TRUE(rounded_translation_vector.isApprox(ground_truth_translation_vector));
}

TEST_F(SideDispenseTest, SideDispenseKabschIdentityTransform) {
    std::vector<Eigen::Vector3d> expected = { 
        {0, 0, 0}, 
        {1, 0, 0}, 
        {0, 1, 0}, 
        {0, 0, 1} 
    };
    std::vector<Eigen::Vector3d> actual = expected;
    auto indices = { 0, 1, 2, 3 };

    DropZoneSearcher::RigidTransformation transform = *DropZoneSearcher::implement_kabsch_rotation_n_translation(expected, actual, indices);

    Eigen::Matrix3d rounded_rotation_matrix = transform.rotation_matrix.unaryExpr([](double x) {
        return (std::abs(x) < 1e-12) ? 0.0 : x;
    });

    Eigen::Vector3d rounded_translation_vector = transform.translation_vector.unaryExpr([](double x) {
        return (std::abs(x) < 1e-12) ? 0.0 : x;
    });

    EXPECT_TRUE(rounded_rotation_matrix.isApprox(Eigen::Matrix3d::Identity(), 1e-9));
    EXPECT_TRUE(rounded_translation_vector.isApprox(Eigen::Vector3d::Zero(), 1e-9));
    EXPECT_NEAR(std::abs(transform.squared_error), 0.0, 1e-9);
    EXPECT_NEAR(transform.rotation_matrix.determinant(), 1.0, 1e-9);
}

TEST_F(SideDispenseTest, SideDispenseKabschPureRotation) {
    std::vector<Eigen::Vector3d> expected = { 
        {1, 0, 0}, 
        {0, 1, 0}, 
        {0, 0, 1}, 
        {1, 1, 1} 
    }; //canonical unit axis points, one diagonal point
    std::vector<Eigen::Vector3d> actual;
    Eigen::AngleAxisd angle(M_PI/4.0, Eigen::Vector3d::UnitZ()); //rotate 45 degrees along z axis
    Eigen::Matrix3d rotation = angle.toRotationMatrix();
    Eigen::Vector3d translation = Eigen::Vector3d::Zero();
    
    for (auto& i : expected) {
        Eigen::Vector3d vec = rotation * i + translation;
        if (std::abs(vec.x()) < 1e-12) {
            vec.x() = 0.0;
        }
        actual.push_back(vec);
    }
    for (auto& i : actual) {
        Logger::Instance()->Debug("Actual point: [{}, {}, {}]", i.x(), i.y(), i.z());
    }
    
    auto indices = { 0, 1, 2, 3 };

    DropZoneSearcher::RigidTransformation transform = *DropZoneSearcher::implement_kabsch_rotation_n_translation(expected, actual, indices);

    Eigen::Matrix3d rounded_rotation_matrix = transform.rotation_matrix.unaryExpr([](double x) {
        return (std::abs(x) < 1e-12) ? 0.0 : x;
    });

    Eigen::Vector3d rounded_translation_vector  = transform.translation_vector.unaryExpr([](double x) {
        return (std::abs(x) < 1e-12) ? 0.0 : x;
    });

    EXPECT_TRUE(rounded_rotation_matrix.isApprox(rotation, 1e-9));
    EXPECT_TRUE(rounded_translation_vector.isApprox(Eigen::Vector3d::Zero(), 1e-9));
    EXPECT_NEAR(transform.squared_error, 0.0, 1e-12);
    EXPECT_NEAR(transform.rotation_matrix.determinant(), 1.0, 1e-9);
}

TEST_F(SideDispenseTest, SideDispenseKabschPureRotationAlongX)
{
    std::vector<Eigen::Vector3d> expected = { 
        {1, 0, 0}, 
        {0, 1, 0}, 
        {0, 0, 1}, 
        {1, 1, 1} 
    }; //canonical unit axis points, one diagonal point
    std::vector<Eigen::Vector3d> actual;
    Eigen::AngleAxisd angle(M_PI/4.0, Eigen::Vector3d::UnitX()); //rotate 45 degrees along x axis
    Eigen::Matrix3d rotation = angle.toRotationMatrix();
    Eigen::Vector3d translation = Eigen::Vector3d::Zero();

    for (auto& i : expected) {
        Eigen::Vector3d vec = rotation * i + translation;
        if (std::abs(vec.x()) < 1e-12) {
            vec.x() = 0.0;
        }
        if (std::abs(vec.y()) < 1e-12) {
            vec.y() = 0.0;
        }
        if (std::abs(vec.z()) < 1e-12) {
            vec.z() = 0.0;
        }
        actual.push_back(vec);
    }

    EXPECT_EQ(actual[0], Eigen::Vector3d(1, 0, 0));
    EXPECT_TRUE(actual[1].isApprox(Eigen::Vector3d(0, std::sqrt(2)/2, std::sqrt(2)/2), 1e-9));
    EXPECT_TRUE(actual[2].isApprox(Eigen::Vector3d(0, -std::sqrt(2)/2, std::sqrt(2)/2), 1e-9));
    EXPECT_TRUE(actual[3].isApprox(Eigen::Vector3d(1, 0, std::sqrt(2)), 1e-9));

    auto indices = { 0, 1, 2, 3 };

    DropZoneSearcher::RigidTransformation transform = *DropZoneSearcher::implement_kabsch_rotation_n_translation(expected, actual, indices);

    Eigen::Matrix3d rounded_rotation_matrix = transform.rotation_matrix.unaryExpr([](double x) {
        return (std::abs(x) < 1e-12) ? 0.0 : x;
    });

    Eigen::Vector3d rounded_translation_vector  = transform.translation_vector.unaryExpr([](double x) {
        return (std::abs(x) < 1e-12) ? 0.0 : x;
    });

    EXPECT_TRUE(rounded_rotation_matrix.isApprox(rotation, 1e-9));
    EXPECT_TRUE(rounded_translation_vector.isApprox(Eigen::Vector3d::Zero(), 1e-9));
    EXPECT_NEAR(transform.squared_error, 0.0, 1e-12);
    EXPECT_NEAR(transform.rotation_matrix.determinant(), 1.0, 1e-9);
}

TEST_F(SideDispenseTest, SideDispenseKabschPureRotationAlongY)
{
    std::vector<Eigen::Vector3d> expected = { 
        {1, 0, 0}, 
        {0, 1, 0}, 
        {0, 0, 1}, 
        {1, 1, 1} 
    }; //canonical unit axis points, one diagonal point
    std::vector<Eigen::Vector3d> actual;
    Eigen::AngleAxisd angle(M_PI/4.0, Eigen::Vector3d::UnitY()); //rotate 45 degrees along y axis
    Eigen::Matrix3d rotation = angle.toRotationMatrix();
    Eigen::Vector3d translation = Eigen::Vector3d::Zero();

    for (auto& i : expected) {
        Eigen::Vector3d vec = rotation * i + translation;
        if (std::abs(vec.x()) < 1e-12) {
            vec.x() = 0.0;
        }
        if (std::abs(vec.y()) < 1e-12) {
            vec.y() = 0.0;
        }
        if (std::abs(vec.z()) < 1e-12) {
            vec.z() = 0.0;
        }
        actual.push_back(vec);
    }

    EXPECT_TRUE(actual[0].isApprox(Eigen::Vector3d(std::sqrt(2)/2, 0, -std::sqrt(2)/2), 1e-9));
    EXPECT_EQ(actual[1], Eigen::Vector3d(0, 1, 0));
    EXPECT_TRUE(actual[2].isApprox(Eigen::Vector3d(std::sqrt(2)/2, 0, std::sqrt(2)/2), 1e-9));
    EXPECT_TRUE(actual[3].isApprox(Eigen::Vector3d(std::sqrt(2), 1, 0), 1e-9));

    auto indices = { 0, 1, 2, 3 };

    DropZoneSearcher::RigidTransformation transform = *DropZoneSearcher::implement_kabsch_rotation_n_translation(expected, actual, indices);

    Eigen::Matrix3d rounded_rotation_matrix = transform.rotation_matrix.unaryExpr([](double x) {
        return (std::abs(x) < 1e-12) ? 0.0 : x;
    });

    Eigen::Vector3d rounded_translation_vector  = transform.translation_vector.unaryExpr([](double x) {
        return (std::abs(x) < 1e-12) ? 0.0 : x;
    });

    EXPECT_TRUE(rounded_rotation_matrix.isApprox(rotation, 1e-9));
    EXPECT_TRUE(rounded_translation_vector.isApprox(Eigen::Vector3d::Zero(), 1e-9));
    EXPECT_NEAR(transform.squared_error, 0.0, 1e-12);
    EXPECT_NEAR(transform.rotation_matrix.determinant(), 1.0, 1e-9);
}

TEST_F(SideDispenseTest, SideDispenseKabschRotationAlongXthenY)
{
    std::vector<Eigen::Vector3d> expected =
    {
        {1, 0, 0},
        {0, 1, 0},
        {0, 0, 1},
        {1, 1, 1}
    }; //canonical unit axis points, one diagonal point

    std::vector<Eigen::Vector3d> actual;

    // angles in radians
    const double deg2rad = M_PI / 180.0;
    double angle_x = -35.0 * deg2rad;   // -35 degrees about X
    double angle_y =  45.0 * deg2rad;   //  45 degrees about Y

    // first rotate about X, then about Y:
    Eigen::AngleAxisd rot_x(angle_x, Eigen::Vector3d::UnitX());
    Eigen::AngleAxisd rot_y(angle_y, Eigen::Vector3d::UnitY());

    // Combined rotation: R = Ry * Rx  (Rx applied first, then Ry)
    Eigen::Matrix3d rotation = (rot_y * rot_x).toRotationMatrix();

    Eigen::Vector3d translation = Eigen::Vector3d::Zero();

    for (auto& i : expected) {
        Eigen::Vector3d vec = rotation * i + translation;
        if (std::abs(vec.x()) < 1e-12) {
            vec.x() = 0.0;
        }
        if (std::abs(vec.y()) < 1e-12) {
            vec.y() = 0.0;
        }
        if (std::abs(vec.z()) < 1e-12) {
            vec.z() = 0.0;
        }
        actual.push_back(vec);
    }

    for (auto& i : actual) {
        Logger::Instance()->Debug("Actual point: [{}, {}, {}]", i.x(), i.y(), i.z());
    }

    EXPECT_TRUE(actual[0].isApprox(Eigen::Vector3d(0.707107, 0, -0.707107), 1e-6));
    EXPECT_TRUE(actual[1].isApprox(Eigen::Vector3d(-0.405580, 0.819152, -0.405580), 1e-6));
    EXPECT_TRUE(actual[2].isApprox(Eigen::Vector3d(0.579228, 0.573576, 0.579228), 1e-6));
    EXPECT_TRUE(actual[3].isApprox(Eigen::Vector3d(0.880755, 1.392728, -0.533459), 1e-6));

    auto indices = { 0, 1, 2, 3 };

    DropZoneSearcher::RigidTransformation transform = *DropZoneSearcher::implement_kabsch_rotation_n_translation(expected, actual, indices);

    Eigen::Matrix3d rounded_rotation_matrix = transform.rotation_matrix.unaryExpr([](double x) {
        return (std::abs(x) < 1e-12) ? 0.0 : x;
    });

    Eigen::Vector3d rounded_translation_vector  = transform.translation_vector.unaryExpr([](double x) {
        return (std::abs(x) < 1e-12) ? 0.0 : x;
    });

    EXPECT_TRUE(rounded_rotation_matrix.isApprox(rotation, 1e-9));
    EXPECT_TRUE(rounded_translation_vector.isApprox(Eigen::Vector3d::Zero(), 1e-9));
    EXPECT_NEAR(transform.squared_error, 0.0, 1e-12);
    EXPECT_NEAR(transform.rotation_matrix.determinant(), 1.0, 1e-9);

}

Eigen::Vector3d calculate_centroid(std::vector<Eigen::Vector3d> points, std::vector<int> indices) {
    Eigen::Vector3d centroid = Eigen::Vector3d::Zero();
    for (int i : indices){
        centroid += points[i];
    }
    centroid = centroid / static_cast<double>(indices.size());
    return centroid;
}

Eigen::Matrix3d compute_cross_covariance(const std::vector<Eigen::Vector3d>& expected, const std::vector<Eigen::Vector3d>& actual, const std::vector<int>& indices) {
    Eigen::Vector3d centroid_a = calculate_centroid(expected, indices);
    Eigen::Vector3d centroid_b = calculate_centroid(actual, indices);
    Eigen::Matrix3d covariance = Eigen::Matrix3d::Zero();

    for (int i : indices) {
        const Eigen::Vector3d centroid_expected = expected[i] - centroid_a;
        const Eigen::Vector3d centroid_actual = actual[i] - centroid_b;
        covariance += centroid_expected * centroid_actual.transpose();
    }

    return covariance;
}

TEST_F(SideDispenseTest, SideDispenseKabschRotationAndTranslation) 
{
    std::vector<Eigen::Vector3d> expected = { 
        {1, 2, 3}, 
        {0, 1, 0}, 
        {4, 5, 6}, 
        {-1, -2, 0} 
    };

    std::vector<Eigen::Vector3d> actual;
    auto indices = { 0, 1, 2, 3 };
    Eigen::AngleAxisd angle(M_PI / 6.0, Eigen::Vector3d(1, 1, 1).normalized());
    Eigen::Matrix3d rotation = angle.toRotationMatrix();
    Eigen::Vector3d translation(3, -2, 5);
    
    for (auto& i : expected) {
        Eigen::Vector3d vec = rotation * i + translation;
        if (std::abs(vec.x()) < 1e-12) {
            vec.x() = 0.0;
        }
        actual.push_back(vec);
    }

    for (auto& i : actual) {
        std::cout << i.x() << i.y() << i.z() << std::endl;
    }

    Eigen::Vector3d centroid_expected = calculate_centroid(expected, indices);
    Eigen::Vector3d centroid_actual = calculate_centroid(actual, indices);

    DropZoneSearcher::RigidTransformation transform = *DropZoneSearcher::implement_kabsch_rotation_n_translation(expected, actual, indices);
    
    Eigen::Matrix3d rounded_rotation_matrix = transform.rotation_matrix.unaryExpr([](double x) {
        return (std::abs(x) < 1e-12) ? 0.0 : x;
    });
    Eigen::Vector3d rounded_translation_vector = transform.translation_vector.unaryExpr([](double x) {
        return (std::abs(x) < 1e-12) ? 0.0 : x;
    });

    EXPECT_TRUE(rounded_rotation_matrix.isApprox(rotation, 1e-8));
    EXPECT_TRUE(rounded_translation_vector.isApprox(translation, 1e-8));
    EXPECT_NEAR(transform.squared_error, 0.0, 1e-10);
    EXPECT_NEAR(transform.rotation_matrix.determinant(), 1.0, 1e-9);
    EXPECT_TRUE(rounded_translation_vector.isApprox(centroid_actual - rounded_rotation_matrix * centroid_expected, 1e-8));

}

TEST_F(SideDispenseTest, SideDispenseKabschCrossCovarianceSVD) 
{
    std::vector<Eigen::Vector3d> expected = { 
        {-1, 0, 0}, 
        {0, 2, 0}, 
        {0, 0, 3}, 
        {1, 1, 1} 
    };

    std::vector<Eigen::Vector3d> actual;
    Eigen::AngleAxisd angle(M_PI / 7.0, Eigen::Vector3d::UnitX());
    Eigen::Matrix3d rotation = angle.toRotationMatrix();
    Eigen::Vector3d translation(0.5, -0.25, 0.75);
    auto indices = { 0, 1, 2, 3 };

    for (auto& i : expected) {
        Eigen::Vector3d vec = rotation * i + translation;
        if (std::abs(vec.x()) < 1e-12) {
            vec.x() = 0.0;
        }
        actual.push_back(vec);
    }
    for (auto& i : actual) {
        std::cout << i.x() << i.y() << i.z() << std::endl;
    }
    
    Eigen::Matrix3d cross_covar = compute_cross_covariance(expected, actual, indices);
    Eigen::JacobiSVD<Eigen::Matrix3d> svd(cross_covar, Eigen::ComputeFullU | Eigen::ComputeFullV);
    Eigen::Matrix3d U = svd.matrixU();
    Eigen::Matrix3d V = svd.matrixV();
    Eigen::Matrix3d svd_rotation = V * U.transpose();

    Eigen::Matrix3d H = U * svd.singularValues().asDiagonal() * V.transpose();
    EXPECT_TRUE(cross_covar.isApprox(H, 1e-10));

    DropZoneSearcher::RigidTransformation transform = *DropZoneSearcher::implement_kabsch_rotation_n_translation(expected, actual, indices);
    
    Eigen::Matrix3d rounded_rotation_matrix = transform.rotation_matrix.unaryExpr([](double x) {
        return (std::abs(x) < 1e-12) ? 0.0 : x;
    });
    Eigen::Vector3d rounded_translation_vector = transform.translation_vector.unaryExpr([](double x) {
        return (std::abs(x) < 1e-12) ? 0.0 : x;
    });
    
    EXPECT_TRUE(rounded_rotation_matrix.isApprox(svd_rotation, 1e-8));
    auto s = svd.singularValues();
    EXPECT_GE(s(0), 0.0);
    EXPECT_GE(s(1), 0.0);
    EXPECT_GE(s(2), 0.0);
    EXPECT_GE(s(0), s(1));
    EXPECT_GE(s(1), s(2));

}



// main to run the tests above
int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
