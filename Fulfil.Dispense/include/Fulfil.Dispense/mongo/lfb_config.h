#pragma once
#include <vector>
#include <Fulfil.Dispense/json.hpp>

namespace fulfil
{
namespace lfb{

class LfbConfig final{//Don't worry this is autogen from FC

    public: 
        std::string LfbGeneration = "3.1";
        double LfbWidth = 0.53;
        double LfbLength = 0.4;
        double LfbBagWidth = 0.45;
        double LfbBagLength = 0.32;
        double LfbCavityHeight = 0.3;
        double InitialRemainingPlatform = 0.21;
        double ContainerWidth = 0.43;
        double ContainerLength = 0.3;
        double EdgeAvoidanceOffset = 0.02;
        double FrontEdgeTargetOffset = 0.03;
        double PortEdgeTargetOffset = 0.03;
        double RearEdgeTargetOffset = 0.03;
        double StarboardEdgeTargetOffset = 0.03;
        double FractionOfBagDimsConsideredInnerBag = 0.8;
        double OnlyConsiderEveryXthTargetCandidate = 5;
        std::vector<double> MarkerCoordinatesX = { 0.23425, -0.23425, -0.23425, -0.248, -0.248, 0.248, 0.248, 0.23425 };
        std::vector<double> MarkerCoordinatesY = { -0.125, -0.125, -0.015, 0.044, 0.125, 0.125, 0.015, -0.044 };
        std::vector<double> MarkerCoordinatesZ = { 0, 0, 0, 0, 0, 0, 0, 0 };
        bool UseYCoordinatesOrientationCheck = true;
        double MarkerAdjustAmount = 0;
        int RegionMaxX = 1150;
        int RegionMinX = 150;
        int RegionMaxY = 700;
        int RegionMinY = 70;
        bool ExtraMarkerValidationRequired = false;
        int MinDim1 = 150;
        int MaxDim1 = 550;
        int MinDim2 = 750;
        int MaxDim2 = 1150;
        bool ExtraCheckInYCoordinates = false;
        bool RotationAllowed = true;
        bool RotateLfbViz = true;
        int NumMarkers = 8;
        int MarkerSize = 4;
        double MarkerDepth = 0.82;
        double MarkerDepthTolerance = 0.5;
        double AllowablePlatformDifference = 0.005;
        double BagFullThresholdMeters = 0.05;
        double MaxItemLengthPercentOverflow = 0.6;
        double AllowedItemOverflowPreDispense = 0.016;
        double AllowedItemOverflowPostDispenseCheck = 0.016;
        int GridRows = 8;
        int GridCols = 6;
        int GridChannels = 3;
        bool AvoidMetalOnMetal = false;
        double DamageBufferWidth = 1;
        double DamageBufferLength = 0;
        double DamageSwingFactor = 0.5;
        int DamageLayersToInclude = 1;
        double DepthBaselineAverageThreshold = 0.02;
        double DepthFactorCorrection = 0.3;
        double DepthTotalThreshold = 0.115;
        double BgSubHistory = 5000;
        double BgSubVarianceThresh = 1000;
        bool BgSubDetectShadows = false;
        int RgbAverageThreshold = 85;
        int RgbTotalThreshold = 150;
        double HLow = 63.72;
        double HHigh = 81.18;
        double SLow = 0;
        double SHigh = 255;
        double VLow = 0;
        double VHigh = 255;
        std::string MongoID = "";
        LfbConfig(nlohmann::json json){
            LfbGeneration = json["LfbGeneration"];
            LfbWidth = json["LfbWidth"];
            LfbLength = json["LfbLength"];
            LfbBagWidth = json["LfbBagWidth"];
            LfbBagLength = json["LfbBagLength"];
            LfbCavityHeight = json["LfbCavityHeight"];
            InitialRemainingPlatform = json["InitialRemainingPlatform"];
            ContainerWidth = json["ContainerWidth"];
            ContainerLength = json["ContainerLength"];
            EdgeAvoidanceOffset = json["EdgeAvoidanceOffset"];
            FrontEdgeTargetOffset = json["FrontEdgeTargetOffset"];
            PortEdgeTargetOffset = json["PortEdgeTargetOffset"];
            RearEdgeTargetOffset = json["RearEdgeTargetOffset"];
            StarboardEdgeTargetOffset = json["StarboardEdgeTargetOffset"];
            FractionOfBagDimsConsideredInnerBag = json["FractionOfBagDimsConsideredInnerBag"];
            OnlyConsiderEveryXthTargetCandidate = json["OnlyConsiderEveryXthTargetCandidate"];
            MarkerCoordinatesX = json["MarkerCoordinatesX"].get<std::vector<double>>();
            MarkerCoordinatesY = json["MarkerCoordinatesY"].get<std::vector<double>>();
            MarkerCoordinatesZ = json["MarkerCoordinatesZ"].get<std::vector<double>>();
            UseYCoordinatesOrientationCheck = json["UseYCoordinatesOrientationCheck"];
            MarkerAdjustAmount = json["MarkerAdjustAmount"];
            RegionMaxX = json["RegionMaxX"];
            RegionMinX = json["RegionMinX"];
            RegionMaxY = json["RegionMaxY"];
            RegionMinY = json["RegionMinY"];
            ExtraMarkerValidationRequired = json["ExtraMarkerValidationRequired"];
            MinDim1 = json["MinDim1"];
            MaxDim1 = json["MaxDim1"];
            MinDim2 = json["MinDim2"];
            MaxDim2 = json["MaxDim2"];
            ExtraCheckInYCoordinates = json["ExtraCheckInYCoordinates"];
            RotationAllowed = json["RotationAllowed"];
            RotateLfbViz = json["RotateLfbViz"];
            NumMarkers = json["NumMarkers"];
            MarkerSize = json["MarkerSize"];
            MarkerDepth = json["MarkerDepth"];
            MarkerDepthTolerance = json["MarkerDepthTolerance"];
            AllowablePlatformDifference = json["AllowablePlatformDifference"];
            BagFullThresholdMeters = json["BagFullThresholdMeters"];
            MaxItemLengthPercentOverflow = json["MaxItemLengthPercentOverflow"];
            AllowedItemOverflowPreDispense = json["AllowedItemOverflowPreDispense"];
            AllowedItemOverflowPostDispenseCheck = json["AllowedItemOverflowPostDispenseCheck"];
            GridRows = json["GridRows"];
            GridCols = json["GridCols"];
            GridChannels = json["GridChannels"];
            AvoidMetalOnMetal = json["AvoidMetalOnMetal"];
            DamageBufferWidth = json["DamageBufferWidth"];
            DamageBufferLength = json["DamageBufferLength"];
            DamageSwingFactor = json["DamageSwingFactor"];
            DamageLayersToInclude = json["DamageLayersToInclude"];
            DepthBaselineAverageThreshold = json["DepthBaselineAverageThreshold"];
            DepthFactorCorrection = json["DepthFactorCorrection"];
            DepthTotalThreshold = json["DepthTotalThreshold"];
            BgSubHistory = json["BgSubHistory"];
            BgSubVarianceThresh = json["BgSubVarianceThresh"];
            BgSubDetectShadows = json["BgSubDetectShadows"];
            RgbAverageThreshold = json["RgbAverageThreshold"];
            RgbTotalThreshold = json["RgbTotalThreshold"];
            HLow = json["HLow"];
            HHigh = json["HHigh"];
            SLow = json["SLow"];
            SHigh = json["SHigh"];
            VLow = json["VLow"];
            VHigh = json["VHigh"];
            std::cout << "LfbConfig MongoID " << MongoID << " --> " << json["MongoID"] << std::endl;
            MongoID = json["MongoID"];

        }

};

}
}