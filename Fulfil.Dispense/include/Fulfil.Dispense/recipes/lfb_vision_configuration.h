//
// Created by jessv on 3/8/24.
//

#ifndef FULFIL_DISPENSE_LFB_VISION_CONFIGURATION_H
#define FULFIL_DISPENSE_LFB_VISION_CONFIGURATION_H

#include <memory>
#include <vector>
#include <json.hpp>

namespace fulfil::configuration::lfb {

    /**
     * Configuration of all LfbVisionConfiguration values, received in the RequestBagState request, sent from FC.
     */
    struct LfbVisionConfiguration {
//        /**
//         * The expected keys that are missing from the request JSON. Will be an empty vector if no expected keys were missing.
//         */
//        std::vector<std::string> missing_keys;

//        /**
//         * Gets the value of the given key in the given JSON. If the key isn't present,
//         * the given default value will be returned, and the key will be added to the missing_keys field.
//         * @tparam T type of the value to get
//         * @param request_json JSON to parse value from
//         * @param key JSON key corresponding to the value in the request_json
//         * @param default_value value to be returned if not found in the JSON
//         * @return value found in the JSON or the default value if missing
//         */
//        template<typename T>
//        T get_value(const nlohmann::json &request_json, std::string key, const T &default_value);

        // metadata
        /** The Mongo ID of the corresponding LfbVisionConfiguration document */
        std::string mongo_id;
        /** The string name of the LFB generation, for example LFB 3.1 */
        std::string lfb_generation {"LFB-3.1"};

        // physical dimensions
        /** The outer width dimension of the physical LFB bot in meters */
        float LFB_width {0.53};
        /** The outer length dimension of the physical LFB bot in meters */
        float LFB_length {0.4};
        /** The outer width dimension of the LFR bag container in meters */
        float LFB_bag_width {0.45};
        /** The outer length dimension of the LFR bag container in meters */
        float LFB_bag_length {0.32};
        /** The height from the bottom of the LFR platform adjustment to the marker surface plane */
        float LFB_cavity_height {0.3};
        /**
         * The width of the Container object used in the drop target searching algorithm, generally smaller than the LFB_bag_width
         */
        float container_width {0.43};
        /**
         * The length of the Container object used in the drop target searching algorithm, generally smaller than the LFB_bag_length
         */
         float container_length {0.3};
        /**
         * Defines the offset from lateral (width direction) and longitudinal (length direction) edges when in nominal
         * orientation of the LFR container for use in creating the Drop Target for empty and nonempty bags.
         * TODO - break into just front and side lol?
         */
        float front_edge_target_offset {0.005};
        float port_edge_target_offset {0};

        // fullness variables
        /**
         * Threshold for average depth in meters of bag contents when LFR platform is bottomed out, for which bag is determined FULL.
         * TODO: REMOVE THIS FUNCTIONALITY AND THIS FIT CHECK! No longer necessary and overly restrictive.
         */
        float bag_full_threshold_meters {0.05};
        // TODO - rename for fraction vs percent?
        /** Percentage of dispensed item length that is allowed to stick out above marker surface after dispense (predicted) */
        float max_item_length_percent_overflow {0.5};
        /** Maximum amount of protrusion in meters that any item can stick out of the bag above marker plane, for items already in the bag */
        float allowed_item_overflow_pre_dispense {0.16};
        /** Maximum amount of protrusion in meters that any item can stick out of the bag above marker plane, for the item to be dispensed */
        float allowed_item_overflow_post_dispense_check {0.16};

        // empty bag handling
        // TODO - need to move out of main.ini
        /**
         * The percentage of the LFB cavity surface that must be non-item in order to verify an empty bag is empty
         */
        int empty_bag_threshold {90};

        // drop target performance
        /**
         * Defines the inner region of the LFR bag container vs. the outer perimeter of the LFR bag container.
         * It is the percentage of width and height to use, where 1 represents 100%.
         */
        float fraction_of_bag_dims_considered_inner_bag {0.85};
        /**
         * After valid candidate regions for drop target are found, this value dictates how to further reduce the
         * candidates to speed up processing for bags that are not empty by skipping all the candidates between
         * the Xth candidates.
         */
        int only_consider_every_Xth_target_candidate {5};
        /**
         * Feature flag to enable treating white materials in the bag as if they are a plastic bag. If true, treat white
         * materials as bag, but if false, treat white materials as items to use in depth processing. The main motivation
         * for having this enabled is to minimize incorrect fullness detected in the LFB as a result of bag crumples and
         * shadows being detected as lots of items. The main motivation for disabling this is to not ignore white large
         * items that are otherwise ignored.
         */
        bool should_filter_out_white {true};
        /**
         * Feature flag for expanding the area around the LFB for depth analysis. If true, the point cloud will be
         * expanded over the LFB walls to include the
         */
        bool extend_depth_analysis_over_markers {true};

        // pirouettes
        /** Feature flag for allowing Drop Target algorithm to provide drop targets that require Pirouette routine */
        bool rotation_allowed {true};
        /**  Feature flag for enabling the rotation of LFB visualizations, allowing Drop Target algorithm to provide drop targets that require Pirouette routine */
        bool rotate_LFB_viz {true};

        // depth detection de-noising - TODO document this better
        float item_protrusion_detection_threshold{0.005};
        int depth_points_above_threshold_to_count_as_item_protruding{5};
        int amount_of_max_depth_points_to_track_for_noise_filtering{8};
        float threshold_depth_difference_to_validate_max_z{0.003};
        int num_points_required_within_valid_distance_to_validate_max_z{5};

        // physical marker variables
        /** Number of Aruco markers/fiducials on top of LFB */
        int num_markers {8};
        /** TODO - size? ; unit: number (defines nxn white squares in the marker region) */
        int marker_size {4};
        /**
         * Minimum number of markers required to be detected for validation purposes
         */
        int min_marker_count_for_validation {3};
        /**
         * Expected depth in meters of Aruco markers in camera coordinate frame. The tolerance really could be a lot smaller here.
         * Used for marker validation. This will change if the distance between the camera and bot change
         */
        float marker_depth {0.82};
        // TODO
        float marker_depth_tolerance {0.5};

        // virtual marker variables
        /**
         * These are the coordinates in meters of the specific feature points of Aruco markers 0 - 7 in the local LFR container frame.
         * Origin is in the center of the LFR container. See Depth Cam diagrams for more information.
         * The specific corner that is used for each marker is specified in marker_detector_container.h as well
         * Usage: Kabsch transform and creation of local containers for local point cloud processing
         */
        std::vector<float> marker_coordinates_x { 0.23425, -0.23425, -0.23425, -0.248, -0.248, 0.248, 0.248, 0.23425 };
        std::vector<float> marker_coordinates_y { -0.125, -0.125, -0.015, 0.044, 0.125, 0.125, 0.015, -0.044 };
        std::vector<float> marker_coordinates_z { 0, 0, 0, 0, 0, 0, 0, 0 };
        /** Maximum x (leftmost) pixel location bound for valid Aruco markers, to ignore neighboring bots in image */
        int region_max_x {1150};
        /** Minimum x (leftmost) pixel location bound for valid Aruco markers, to ignore neighboring bots in image */
        int region_min_x {150};
        /** Maximum y (UPmost TODO) pixel location bound for valid Aruco markers, to ignore neighboring bots in image */
        int region_max_y {700};
        /** Minimum y (DOWNmost TODO) pixel location bound for valid Aruco markers, to ignore neighboring bots in image */
        int region_min_y {70};
        /**
         *  Minimum dimension pixel value for markers: 1, 2, 3, 4  (for nominal bot rotation status). Used for confirming
         *  bot location in comparison to camera
         */
        int min_dim_1 {150};
        /**
         * Maximum dimension pixel value for markers: 1, 2, 3, 4  (for nominal bot rotation status). Used for confirming
         *  bot location in comparison to camera
         */
        int max_dim_1 {550};
        /**
         * Minimum dimension pixel value for markers: 0, 5, 6, 7  (for nominal bot rotation status). Used for confirming
         *  bot location in comparison to camera
         */
        int min_dim_2 {750};
        /**
         * Maximum dimension pixel value for markers: 0, 5, 6, 7  (for nominal bot rotation status). Used for confirming
         *  bot location in comparison to camera
         */
        int max_dim_2 {1150};
        /**
         * This boolean sets whether to use the Y pixel coordinates of detected markers on the bot to do an orientation check on the bot
         * during Drop Target algorithm processing. If set to false, X coordinates are used.
         * The historical need for this flag was to use different pixel values for different bot versions (LFR2 vs. LFR3) which were
         * themselves rotated from each other. This will always be the same for a given bot type.
         */
        bool use_y_coordinates_orientation_check {true};
        /**
         * Feature flag for a more extensive marker validation location check to confirm bot location in comparison to camera
         */
        bool extra_marker_validation_required {false};
        /**
         * Dictates which axis (y or x) is used for the extra marker validation check enabled by the `extra_marker_validation_required` feature flag
         * TRUE: extra dimension check in y pixel coordinates, FALSE: in x pixel coordinates.
         */
        bool extra_check_in_y_coordinates {false};

        // drop depth grid
        /**
         * Number of rows in the drop depth grid TODO - clarify
         */
        int num_rows_in_drop_depth_grid {22};
        /**
         * Number of columns in the drop depth grid TODO - clarify
         */
        int num_cols_in_drop_depth_grid {15};

        // Drop Grid and Mongo Bag State
        /**
         * Number of rows in the bag grid map for bag state tracking (for drop target analysis and damage assessment). TODO compare with drop depth grid, rename
         */
        int grid_rows {8};
        /**
         * Number of columns in the bag grid map for bag state tracking (for drop target analysis and damage assessment).
         */
        int grid_cols {6};
        /**
         * Number of layers of the bag grid map used in bag state tracking. WARNING: Current Bag State handling ONLY
         * allows up to 3 channels, do not increase beyond 3.
         */
        int grid_channels {3}; // TODO Should this be the same as damage_layers_to_include?
        /**
         * TODO - ??? are any errors thrown by this?
         * why 1250 default of 25 * 35 = 1250?
         * Maximum number of depth detections in the LFB container by the point cloud, when there are no missing depth data points
         * (will vary some from image/image, bay/bay, etc.)
         */
        int max_num_depth_detections {1250};

        // drop target damage risk assessment
        /** Feature flag for avoiding dispensing metal items onto metal items in the bag. */
        bool avoid_metal_on_metal {true};
        // TODO
        float mass_threshold_extra_fragile {150};
        /** Number of bag rejections resulting from damage avoidance allowed before the bag is sent to pickup. */
        int max_allowable_damage_rejections {2};
        ;// These values indicate how Bag State damage risk assessment occurs. The risk assessment highlights grid regions of the bag
        ;// in red that contain damage risk items. And yellow regions where a center of a drop target for the dispensed item would lead
        ;// to the item hitting a red risk region. The damage_swing_factor is included in that calculation.
        ;// layers_to_include indicates how many layers deep items are considered as risky based on their damage risk codes.
        ;// damage buffer width and length increase the yellow keep out region beyond the original calculation by a certain number of
        ;// squares on each side, in alignment with the bot width or length.
        ;// Frequency of change: on occasion the buffer length and width are changed to be more conservative / liberal in avoiding damage interactions
        int damage_buffer_width {1}; //for use in assessing damage risk in mongo_bag_state // TODO is int?
        float damage_buffer_length {0}; // TODO is float?
        float damage_swing_factor {0.5};
        /** Number of damage layers to include in damage avoidance processing, cannot be more than 3 */
        int damage_layers_to_include {1};

        // post-drop item-in-bag sensor detection
        //TODO these may need a rename bc unclear
        ;// These values define how the Pre/Post image Drop Grids average depths are compared and whether an item is determined to have been dispensed.
        ;// The comparison for each individual grid cell must surpass the baseline_threshold or a depth_factor_correction times the item's dimension,
        ;// whichever is HIGHER. The logic there is that for large items, the depth difference should be expected to be larger if an item is present.
        ;// There is also a global comparison across the entire grid, and that is controlled by depth_total_threshold.
        ;// Frequency of change: COMMON! This is the primary method of optimizing the item-in-bag sensor detection, depth comparison
        float depth_baseline_average_threshold {0.02}; ;//for use in depth pre-post comparison processing
        float depth_factor_correction {0.3};
        float depth_total_threshold {0.115};

        // post-drop RGB channel comparisons
/** These values define how the Pre/Post image RGB channel comparisons occur.
         * This comparison is only used if the depth comparison is inconclusive, so it is not changed as frequently. TODO
         */
        int bg_sub_history {5000};
        /**
         * must be double for usage in cv algo
         */
        double bg_sub_variance_threshold {1000};     ; ;//Threshold on the squared distance between the pixel and the sample to decide whether a pixel is close to that sample
        /**
         * TODO
         * If true, the algorithm will detect shadows and mark them. It decreases the speed a bit, so if you do not need this feature, set the parameter to false.
         */
        bool bg_sub_detect_shadows {false};
        int RGB_average_threshold {85}; // TODO float? or int?
        int RGB_total_threshold {150};

        // pre-drop/post-drop comparison
        /**
         * Acceptable difference in meters in Remaining_Platform input from FC for the Pre/Post image comparison.
         * Increasing this will affect the item-in-bag sensor accuracy
         */
        float allowable_platform_difference {0.005};

        // HSV values
        /** HSV-Hue low value - must be double for cv::Scalar usage */
        double H_low {63.72};
        /** HSV-Hue high value - must be double for cv::Scalar usage */
        double H_high {81.18};
        /** HSV-Saturation low value - must be double for cv::Scalar usage */
        double S_low {0};
        /** HSV-Saturation high value - must be double for cv::Scalar usage */
        double S_high {255};
        /** HSV-Value low value - must be double for cv::Scalar usage */
        double V_low {0};
        /** HSV-Value high value - must be double for cv::Scalar usage */
        double V_high {255};

        LfbVisionConfiguration() = default;
        LfbVisionConfiguration(const std::shared_ptr<nlohmann::json>& input_json);
    };

} // namespace fulfil::configuration::lfb


#endif //FULFIL_DISPENSE_LFB_VISION_CONFIGURATION_H
