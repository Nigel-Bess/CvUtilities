#pragma once
#include <memory>
#include <Fulfil.Dispense/drop/point_cloud_split_result.h>
#include <Fulfil.Dispense/drop/occupancy_debug_data.h>
#include <Fulfil.CPPUtils/commands/dc_api_error_codes.h>

using std::shared_ptr;
using std::vector;

struct SideDispenseOccupancyResult {
  DcApiErrorCode error_code;
  shared_ptr<vector<shared_ptr<vector<float>>>> occupancy_map;
  shared_ptr<PointCloudSplitResult> split;
  shared_ptr<OccupancyDebugData> debug_data;
};


