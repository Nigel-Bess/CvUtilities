////
//// Created by jessv on 3/8/24.
////

#include <memory>
#include <string>
#include <vector>
#include <Fulfil.CPPUtils/logging.h>
#include "Fulfil.Dispense/recipes/lfb_vision_configuration.h"

using fulfil::configuration::lfb::LfbVisionConfiguration;
using fulfil::utils::Logger;

LfbVisionConfiguration::LfbVisionConfiguration(const std::shared_ptr<nlohmann::json>& input_json) :

    // NOTE: The currently defined defaults are for LFB 3.1 as of March 2024.
	config_json{input_json},

    // metadata
    mongo_id{input_json->value("MongoID", std::string("999999999999999999999999"))},
    lfb_generation{input_json->value("LfbGeneration", "LFB-3.1")},

    // physical dimensions
    is_side_dispense{input_json->value("IsSideDispense", false)},
    LFB_width{input_json->value("LfbWidthMeters", 0.53F)},
    LFB_length{input_json->value("LfbLengthMeters", 0.4F)},
    LFB_bag_width{input_json->value("LfbBagWidthMeters",   0.45F)},
    LFB_bag_length{input_json->value("LfbBagLengthMeters", (float)0.32)},
    LFB_cavity_height{input_json->value("LfbCavityHeightMeters", (float)0.30)},
    container_width{input_json->value("ContainerWidthMeters", (float)0.43)},
    container_length{input_json->value("ContainerLengthMeters", (float)0.30)},
    front_edge_target_offset{input_json->value("WidthWiseEdgeTargetOffsetMeters", (float)0.005)},
    port_edge_target_offset{input_json->value("LengthWiseEdgeTargetOffsetMeters", (float)0.0)},

    // fullness variables
    bag_full_threshold_meters{input_json->value("BagFullThresholdMeters", (float)0.05)},
    // TODO - rename for fraction vs percent?
    max_item_length_percent_overflow{input_json->value("MaxItemLengthPercentOverflow", (float)0.5)},
    allowed_item_overflow_pre_dispense{input_json->value("AllowedItemOverflowPreDispenseMeters", (float)0.016)},
    allowed_item_overflow_post_dispense_check{input_json->value("AllowedItemOverflowPostDispenseMeters", (float)0.016)},

    // empty bag handling
    empty_bag_threshold{input_json->value("PercentageThresholdOfCavityIsItemlessToBeEmpty", 90)},

    // drop target performance
    fraction_of_bag_dims_considered_inner_bag{input_json->value("FractionOfBagDimsConsideredInnerBag", (float)0.85)},
    only_consider_every_Xth_target_candidate{input_json->value("OnlyConsiderEveryXthTargetCandidate", 5)},
    should_filter_out_white{input_json->value("ShouldFilterOutWhite", false)},
    extend_depth_analysis_over_markers{input_json->value("ExtendDepthAnalysisOverMarkers", true)},

    // pirouettes
    rotation_allowed{input_json->value("RotationAllowed", true)},
    rotate_LFB_viz{input_json->value("RotateLfbViz", true)},

    // depth detection de-noising
    item_protrusion_detection_threshold{input_json->value("ItemProtrusionDetectionThresholdMeters", 0.005F)},
    depth_points_above_threshold_to_count_as_item_protruding{input_json->value("DepthPointsAboveThresholdToCountAsItemProtruding", 5)},
    amount_of_max_depth_points_to_track_for_noise_filtering{input_json->value("AmountOfMaxDepthPointsToTrackForNoiseFiltering", 8)},
    threshold_depth_difference_to_validate_max_z{input_json->value("ThresholdDepthDifferenceToValidateMaxZMeters", 0.003F)},
    num_points_required_within_valid_distance_to_validate_max_z{input_json->value("NumPointsRequiredWithinValidDistanceToValidateMaxZ", 5)},

	// physical antenna variables
	antenna_x_distance_to_container_edge_meters{input_json->value("AntennaXDistanceToContainerEdgeMeters", 0.0175F)},
	antenna_y_distance_to_container_edge_meters{input_json->value("AntennaYDistanceToContainerEdgeMeters", 0.070F)},
	antenna_omission_buffer_meters{input_json->value("AntennaOmissionBufferMeters", 0.005F)},

    // physical marker variables
    num_markers{input_json->value("NumMarkers", 8)}, // TODO - this isn't fully configurable just by changing this
    min_marker_count_for_validation{input_json->value("MinMarkerCountForValidation", 3)},
    marker_size{input_json->value("MarkerSize", 4)},
    marker_depth{input_json->value("MarkerDepthMeters", (float)0.82)},
    marker_depth_tolerance{input_json->value("MarkerDepthToleranceMeters", (float)0.5)},

    // it is used to remove high depth points from the local point cloud
    threshold_above_highest_valid_depth{input_json->value("DepthThresholdAboveValidDepthMeters", (float)0.15)},

    // virtual marker variables
    marker_coordinates_x{input_json->value("MarkerCoordinatesX", std::vector<float>({ 0.23425, -0.23425, -0.23425, -0.248, -0.248, 0.248, 0.248, 0.23425 }))},
    marker_coordinates_y{input_json->value("MarkerCoordinatesY", std::vector<float>({ -0.125, -0.125, -0.015, 0.044, 0.125, 0.125, 0.015, -0.044 }))},
    marker_coordinates_z{input_json->value("MarkerCoordinatesZ", std::vector<float>({ 0, 0, 0, 0, 0, 0, 0, 0 }))},
    region_max_x{input_json->value("RegionMaxXPixels", 1150)},
    region_min_x{input_json->value("RegionMinXPixels", 150)},
    region_max_y{input_json->value("RegionMaxYPixels", 700)},
    region_min_y{input_json->value("RegionMinYPixels", 70)},
    min_dim_1{input_json->value("MinDim1Pixels", 150)},
    min_dim_2{input_json->value("MinDim2Pixels", 750)},
    max_dim_2{input_json->value("MaxDim2Pixels", 1150)},
    use_y_coordinates_orientation_check{input_json->value("UseYCoordinatesOrientationCheck", true)},
    extra_marker_validation_required{input_json->value("ExtraMarkerValidationRequired", false)},
    extra_check_in_y_coordinates{input_json->value("ExtraCheckInYCoordinates", false)},

    // drop depth grid
    num_rows_in_drop_depth_grid{input_json->value("NumRowsInDropDepthGrid", 22)},
    num_cols_in_drop_depth_grid{input_json->value("NumColumnsInDropDepthGrid", 15)},

    // Drop Grid and Mongo Bag State
    grid_rows{input_json->value("GridRows", 8)},
    grid_cols{input_json->value("GridCols", 6)},
    grid_channels{input_json->value("GridChannels", 3)}, // TODO Should this be the same as damage_layers_to_include? also this var seems super under-utilized in general
    max_num_depth_detections{input_json->value("MaxNumberOfDepthDetections", 1250)},

    // drop target damage risk assessment
    avoid_metal_on_metal{input_json->value("AvoidMetalOnMetal", true)},
    max_allowable_damage_rejections{input_json->value("MaxAllowableDamageRejections", 2)},
    mass_threshold_extra_fragile{input_json->value("MassThresholdExtraFragileGrams", (float)150.0)},
    damage_buffer_width{input_json->value("DamageBufferWidth", 1)}, // TODO is int?
    damage_buffer_length{input_json->value("DamageBufferLength", 0.0F)}, // TODO is float?
    damage_swing_factor{input_json->value("DamageSwingFactor", (float)0.5)},
    damage_layers_to_include{input_json->value("DamageLayersToInclude", 1)},

    // post-drop item-in-bag sensor detection
    //    These values define how the Pre/Post image Drop Grids average depths are compared and whether an item is determined to have been dispensed.
    //;// The comparison for each individual grid cell must surpass the baseline_threshold or a depth_factor_correction times the item's dimension,
    //;// whichever is HIGHER. The logic there is that for large items, the depth difference should be expected to be larger if an item is present.
    //;// There is also a global comparison across the entire grid, and that is controlled by depth_total_threshold.
    //;// Frequency of change: COMMON! This is the primary method of optimizing the item-in-bag
    depth_baseline_average_threshold{input_json->value("DepthBaselineAverageThresholdMeters", (float)0.02)},
    depth_factor_correction{input_json->value("DepthFactorCorrection", (float)0.3)},
    depth_total_threshold{input_json->value("DepthTotalThreshold", (float)0.115)},

    // post-drop RGB channel comparisons
    bg_sub_history{input_json->value("BgSubHistory", 5000)},
    bg_sub_variance_threshold{input_json->value("BgSubVarianceThreshold", (float)1000.0)},
    bg_sub_detect_shadows{input_json->value("BgSubDetectShadows", false)},
    RGB_average_threshold{input_json->value("RgbAverageThreshold", 85)},
    RGB_total_threshold{input_json->value("RgbTotalThreshold", 150)},

    // pre-drop/post-drop comparison
    allowable_platform_difference{input_json->value("AllowablePlatformDifferencePreVsPostDispenseMeters", (float)0.005)},

    // HSV values
    H_low{input_json->value("HsvHueLow", (float)63.72)},
    H_high{input_json->value("HsvHueHigh", (float)81.18)},
    S_low{input_json->value("HsvSaturationLow", (float)0.0)},
    S_high{input_json->value("HsvSaturationHigh", (float)255.0)},
    V_low{input_json->value("HsvValueLow", (float)0.0)},
    V_high{input_json->value("HsvValueHigh", (float)255.0)}
{}
