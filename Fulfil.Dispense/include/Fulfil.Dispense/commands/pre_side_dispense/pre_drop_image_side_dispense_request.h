#pragma once
#include <string>
#include <vector>
#include <array>
#include "aruco_tag.h"
#include <Fulfil.CPPUtils/math/rigid_transformation.h>

struct PreDropImageSideDispenseRequest {
    int Type;
    std::string PrimaryKeyId;
    int OccupancyMapWidth;
    int OccupancyMapHeight;
    bool IsEmptyBag;
    Eigen::Vector3d BagCavityDimensions;
    std::vector<ArucoTag> ArucoTags;
    bool RequestOccupancyVisualization;
    RigidTransformation ExpectedBotPose;
};

inline void from_json(const nlohmann::json& j, PreDropImageSideDispenseRequest& r)
{
    j.at("Type").get_to(r.Type);
    j.at("Primary_Key_ID").get_to(r.PrimaryKeyId);
    j.at("Occupancy_Map_Width").get_to(r.OccupancyMapWidth);
    j.at("Occupancy_Map_Height").get_to(r.OccupancyMapHeight);
    j.at("Is_Empty_Bag").get_to(r.IsEmptyBag);
    j.at("Bag_Cavity_Dimensions").get_to(r.BagCavityDimensions);
    j.at("Aruco_Tags").get_to(r.ArucoTags);
    j.at("Request_Occupancy_Visualization").get_to(r.RequestOccupancyVisualization);
    j.at("Expected_Bag_Cavity_Transform").get_to(r.ExpectedBotPose);
}