#pragma once
#include <memory>
#include <vector>
#include <Fulfil.CPPUtils/math/rigid_transformation.h>

using std::shared_ptr;
using std::vector;
using Eigen::Vector3d;
using Eigen::Vector3i;

struct OccupancyDebugData {
    struct BagCavityOpening
    {
        RigidTransformation Expected;
        RigidTransformation Actual;
    } BagCavityOpening;
    Vector3d BagCavityDimensionsMm;
    struct CompletePointCloud
    {
        shared_ptr<vector<Vector3i>> InsideBagCavity; // integers to reduce data bloat in json (decimal precision not needed)
        shared_ptr<vector<Vector3i>> OutsideBagCavity; // integers to reduce data bloat in json (decimal precision not needed)
    } PointCloud;
    struct ArucoLocations
    {
        shared_ptr<vector<Vector3d>> Expected;
        shared_ptr<vector<Vector3d>> Actual;
        shared_ptr<vector<Vector3d>> Matched;
    } ArucoLocations;
    
};

inline nlohmann::json to_json(const OccupancyDebugData& data)
{
    nlohmann::json j;

    j["bag_cavity_opening"] = {
        {"expected", nlohmann::json(data.BagCavityOpening.Expected)},
        {"actual",   nlohmann::json(data.BagCavityOpening.Actual)}
    };

    j["bag_cavity_dimensions_mm"] =
        nlohmann::json(data.BagCavityDimensionsMm);

    j["point_cloud"] = {
        {"inside_bag_cavity",
         nlohmann::json(*data.PointCloud.InsideBagCavity)},
        {"outside_bag_cavity",
         nlohmann::json(*data.PointCloud.OutsideBagCavity)}
    };

    j["aruco_locations"] = {
        {"expected", nlohmann::json(*data.ArucoLocations.Expected)},
        {"actual",   nlohmann::json(*data.ArucoLocations.Actual)},
        {"matched",  nlohmann::json(*data.ArucoLocations.Matched)}
    };

    return j;
}



