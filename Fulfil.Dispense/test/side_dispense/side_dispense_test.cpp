#include <gtest/gtest.h>
#include <Fulfil.DepthCam/mocks/mock_session.h>
#include <Fulfil.DepthCam/aruco/marker_detector_container.h>
#include <Fulfil.DepthCam/point_cloud/local_point_cloud.h>
#include <Fulfil.Dispense/drop/drop_zone_searcher.h>
#include <Fulfil.Dispense/drop/drop_manager.h>
#include <Fulfil.CPPUtils/inih/INIReader.h>
#include <Fulfil.CPPUtils/conversions.h>
#include <Fulfil.Dispense/visualization/make_media.h>
#include <FulfilMongoCpp/mongo_connection.h>
#include <Fulfil.Dispense/recipes/lfb_vision_configuration.h>
#include <Fulfil.CPPUtils/file_system_util.h>

using fulfil::depthcam::mocks::MockSession;
using fulfil::depthcam::aruco::MarkerDetectorContainer;
using fulfil::dispense::drop::DropZoneSearcher;
using ff_mongo_cpp::MongoConnection;
using fulfil::configuration::lfb::LfbVisionConfiguration;
using std::vector;
using Eigen::Vector3d;
using fulfil::utils::FileSystemUtil;



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

    std::shared_ptr<nlohmann::json> pre_json_request;

    std::shared_ptr<LfbVisionConfiguration> pre_lfb_vision_config;
    std::shared_ptr<LfbVisionConfiguration> post_lfb_vision_config;

    std::shared_ptr<INIReader> dispense_man_config;


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

        pre_json_request = read_in_json(*pre_test_data_directory + std::string("/2025_09_24_H21_M35_S33"), "json_request.json");

        dispense_man_config = std::make_shared<INIReader>(*pre_test_data_directory + std::string("/main_config.ini"), false, true);

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

    ASSERT_NE(pre_json_request, nullptr);

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

vector<ArucoTagMatch> generate_tag_matches(vector<Vector3d> aruco_locations_at_identity_transform, vector<Vector3d> measured_aruco_locations){
    auto n_arucos = aruco_locations_at_identity_transform.size();
    vector<ArucoTagMatch> aruco_matches(n_arucos);
    for(int i = 0; i < n_arucos; i++)
    {
        aruco_matches[i] = ArucoTagMatch{ArucoTag{aruco_locations_at_identity_transform[i],i},measured_aruco_locations[i]};
    }
    return aruco_matches;
}

void test_pose_estimation(vector<Vector3d> aruco_locations_at_identity_transform, vector<Vector3d> measured_aruco_locations, Eigen::Matrix3d ground_truth_rotation_matrix, Vector3d ground_truth_translation_vector){
    auto aruco_matches = generate_tag_matches(aruco_locations_at_identity_transform, measured_aruco_locations);
    BotPoseEstimationResult estimationResult = *DropZoneSearcher::estimate_bot_pose(aruco_matches);
    RigidTransformation transform = estimationResult.Transform;
    Eigen::Matrix3d rounded_rotation_matrix = transform.Rotation.unaryExpr([](double x) {
        return (std::abs(x) < 1e-12) ? 0.0 : x;
    });

    Eigen::Vector3d rounded_translation_vector = transform.Translation.unaryExpr([](double x) {
        return (std::abs(x) < 1e-12) ? 0.0 : x;
    });

    EXPECT_TRUE(rounded_rotation_matrix.isApprox(ground_truth_rotation_matrix));
    EXPECT_TRUE(rounded_translation_vector.isApprox(ground_truth_translation_vector));

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

    Eigen::Matrix3d ground_truth_rotation_matrix = Eigen::Matrix3d::Identity();
    Eigen::Vector3d ground_truth_translation_vector(-20, 0, 0);
    test_pose_estimation(aruco_locations_at_identity_transform, measured_aruco_locations, ground_truth_rotation_matrix, ground_truth_translation_vector);
    
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
    Eigen::Matrix3d ground_truth_rotation_matrix = Eigen::Matrix3d::Identity();
    Eigen::Vector3d ground_truth_translation_vector(-20, 15, 0);
    test_pose_estimation(aruco_locations_at_identity_transform, measured_aruco_locations, ground_truth_rotation_matrix, ground_truth_translation_vector);    
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

    Eigen::Matrix3d ground_truth_rotation_matrix = Eigen::Matrix3d::Identity();
    Eigen::Vector3d ground_truth_translation_vector(-20, 15, 5);
    test_pose_estimation(aruco_locations_at_identity_transform, measured_aruco_locations, ground_truth_rotation_matrix, ground_truth_translation_vector);
}

TEST_F(SideDispenseTest, SideDispenseKabschIdentityTransform) {
    std::vector<Eigen::Vector3d> at_identity = { 
        {0, 0, 0}, 
        {1, 0, 0}, 
        {0, 1, 0}, 
        {0, 0, 1} 
    };
    std::vector<Eigen::Vector3d> actual = at_identity;
    test_pose_estimation(at_identity, actual, Eigen::Matrix3d::Identity(), Eigen::Vector3d::Zero());
}


// main to run the tests above
int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
