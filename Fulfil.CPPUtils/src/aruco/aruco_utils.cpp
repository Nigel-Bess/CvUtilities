#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>
#include <opencv2/features2d.hpp>
#include <algorithm>
#include <limits.h>
#include "Fulfil.CPPUtils/aruco/aruco_utils.h"
#include <numeric>
#include <Fulfil.CPPUtils/logging.h>
#include <filesystem>

using fulfil::utils::aruco::ArucoTransforms;
using fulfil::utils::aruco::ImageMarkers;
using fulfil::utils::aruco::HomographyResult;
using fulfil::utils::aruco::BestMarkers;
using fulfil::utils::Logger;

const auto EMPTY_POINT_ARRAY = std::make_shared<std::vector<std::shared_ptr<std::vector<std::shared_ptr<cv::Point2f>>>>>();
const auto EMPTY_INT_ARRAY = std::make_shared<std::vector<int>>();
const auto EMPTY_IMG_MARKERS = std::make_shared<ImageMarkers>(EMPTY_POINT_ARRAY, EMPTY_INT_ARRAY);

ArucoTransforms::ArucoTransforms(
    cv::Ptr<cv::aruco::Dictionary> dict,
    std::shared_ptr<cv::aruco::DetectorParameters> params,
    double maxMarkerDisplacementDist,
    int minCandidateMarkerMatches,
    int maxCandidateMarkerMatches)
{
    this->baselineCandidates = new std::map<std::string, std::shared_ptr<ImageMarkers>>();
    this->dict = dict;
    this->params = params;
    this->maxMarkerDisplacementDist = maxMarkerDisplacementDist;
    this->minCandidateMarkerMatches = minCandidateMarkerMatches;
    this->maxCandidateMarkerMatches = maxCandidateMarkerMatches;
}

std::shared_ptr<ImageMarkers> ArucoTransforms::detectArucoMarkers(cv::Mat img, std::string candidateName) {
    try
    {
        std::vector<std::vector<cv::Point2f>> corners;
        auto ids = std::make_shared<std::vector<int>>();
        std::vector<std::vector<cv::Point2f>> rejected;
        rejected.reserve(100);
        cv::aruco::detectMarkers(img, this->dict, corners, *ids, this->params, rejected);
        if (rejected.size() > 0) {
            Logger::Instance()->Info("[Aruco]: Rejected {} markers, accepted {}", rejected.size(), corners.size());
        }
        
        auto sharedCorners = std::make_shared<std::vector<std::shared_ptr<std::vector<std::shared_ptr<cv::Point2f>>>>>();
        sharedCorners->reserve(100);
        for (int i = 0; i < corners.size(); i++) {
            sharedCorners->push_back(std::make_shared<std::vector<std::shared_ptr<cv::Point2f>>>());
            for (int j = 0; j < 4; j++) {
                (*sharedCorners)[i]->push_back(std::make_shared<cv::Point2f>(((corners)[i][j]).x, ((corners)[i][j]).y));
            }
        }
        Logger::Instance()->Info("[Aruco]: detectArucoMarkers done", rejected.size(), corners.size());

        
        return std::make_shared<ImageMarkers>(sharedCorners, ids);
    }
    catch (const std::exception &e)
    {
        Logger::Instance()->Error("[Aruco]: Error in drawDetectedMarkers! ");
    }
    return EMPTY_IMG_MARKERS;
}

double ArucoTransforms::calculateMarkerAvgDist(std::shared_ptr<ImageMarkers> candidate, std::shared_ptr<std::vector<std::shared_ptr<cv::Point2f>>> markerCorners, int markerId) {
    auto ids = candidate->ids;
    int candidateMarkerIndex = 0;
    for (; markerId != (*ids)[candidateMarkerIndex]; candidateMarkerIndex++) {}
    auto candidateCorners = (*candidate->markers)[candidateMarkerIndex];
    double distSum = 0;
    for (int i = 0; i < 4; i++) {
        auto dist = distance((*candidateCorners)[i], (*markerCorners)[i]);
        distSum += dist;
    }
    return distSum / 4.0d; // Average dist over all 4 points
}

std::shared_ptr<BestMarkers> ArucoTransforms::findBestMarkers(cv::Mat targetImage, std::shared_ptr<ImageMarkers> candidate, std::shared_ptr<ImageMarkers> instance) {
    auto cids = candidate->ids;
    // First find all best tag matches and key them by marker IDs
    std::map<int, std::shared_ptr<std::vector<std::shared_ptr<cv::Point2f>>>> id_to_best_fit;
    std::map<int, double> id_to_dist_sum;
    int mCount = instance->markers->size();
    bool baselineFound = false;
    for (int i = 0; i < mCount; i++) {
        auto corners = (*instance->markers)[i];
        auto id = (*instance->ids)[i];
        // Ignore IDs not shared with candidate
        if (std::find(cids->begin(), cids->end(), id) != cids->end()) {
            bool writeThisID = false;
            // cur_dist is normalized from [0, 1]
            double cur_dist = calculateMarkerAvgDist(candidate, corners, id) / (double)(std::max(targetImage.size().width, targetImage.size().height));
            // If ID already seen, take the one with the smallest candidate diff
            if (id_to_best_fit.find(id) != id_to_best_fit.end()) {
                double last_dist = 99999999;
                if (id_to_dist_sum.find(id) != id_to_dist_sum.end()) {
                    last_dist = id_to_dist_sum.at(id);
                }
                if (cur_dist < last_dist) {
                    writeThisID = true;
                }
            } else {
                writeThisID = true;
            }
            if (writeThisID) {
                baselineFound = true;
                id_to_dist_sum.insert(std::make_pair(id, cur_dist));
                id_to_best_fit.insert(std::make_pair(id, corners));
            }
        }
    }
    // Convert map to flat array and sort by shortest diff distances
    auto markers = std::make_shared<std::vector<std::shared_ptr<std::vector<std::shared_ptr<cv::Point2f>>>>>();
    auto ids = std::make_shared<std::vector<int>>();
    std::vector<double> distances;
    distances.reserve(candidate->ids->size() * 2 * 4);
    for (auto [id, corners] : id_to_best_fit)
    {
        markers->push_back(corners);
        // Find candidate marker that matches by id
        ids->push_back(id);
        distances.push_back(id_to_dist_sum.at(id));
    }
    // Sort markers and ids vectors by id_to_dist_sum
    // Shame Eric for a lazy bubble sort here since N will never be > perfect candidate marker count
    for (int i = 0; i < mCount; i++) {
        for (int j = 0; j < mCount; j++) {
            if (i == j) continue;
            auto iDist = distances[i];
            auto jDist = distances[j];
            if ((i < j && iDist > jDist) || (i > j && iDist < jDist)) {
                //(&(*markers)[i])->swap((*markers)[j]);
                //std::swap(&(*ids)[i], &(*ids)[j]);
                //std::swap(&(distances[i]), &(distances[j]));
            }
        }
    }
    // Distance score is the sum of distances between candidate marker corners (normalized by point count) and
    // current img marker corners except candidate markers missing from cur image are penalized
    // by the worst-case 4 corner marker diff so that candidates with less matching markers are discouraged
    double distance_score = DBL_MAX / 2;
    if (distances.size() > 0) {
        double dist_sum = 0.0;
        for (auto &dist : distances) {
            dist_sum += dist;
        }
        distance_score = dist_sum / distances.size();
    }
    // Penalize missing IDs by max distance of 1.0, must match IDs 1-to-1
    int missing_count = cids->size();
    for (const auto &id : *ids) {
        bool found = false;
        for (const auto &cid : *cids) {
            if (cid == id) {
                found = true;
                break;
            }
        }
        if (found) {
            missing_count--;
        }
    }

    Logger::Instance()->Info("[Aruco]: db 1 {}", cids->size());
    Logger::Instance()->Info("[Aruco]: db 2 {}", ids->size());
    Logger::Instance()->Info("[Aruco]: db 3 {}", instance->ids->size());

    Logger::Instance()->Info("[Aruco]: Candidate was missing {} tags", missing_count);
    // Penalize missing IDs by 4 / N baseline markers, roughly equal to worse-case point distances for 4 corner points
    distance_score += (missing_count * 4.0) / candidate->ids->size();
    auto to_return = std::make_shared<BestMarkers>(std::make_shared<ImageMarkers>(markers, ids), distance_score);
    return to_return;
}

void ArucoTransforms::loadBaselineImageAsCandidate(std::string srcImgFile, std::string name, int expectedMarkerCount) {
    if (!std::filesystem::exists(srcImgFile)) {
        throw std::runtime_error("[Aruco]: Failed to find baseline image " + srcImgFile);
    }

    cv::Mat image = cv::imread(srcImgFile, cv::IMREAD_COLOR);
    auto imgMarkers = detectArucoMarkers(image, name);
    std::string img_path = "/home/fulfil/code/Fulfil.ComputerVision/Fulfil.TCS/data/test/image/" + name + ".jpeg";
    cv::imwrite(img_path, image);

    // Uncomment to view all loaded basline images for debugging purposes in data/test
    auto markedFile = "Fulfil.TCS/data/test/" + name + "_marked_" + std::to_string(expectedMarkerCount) + ".jpeg";
    std::vector<std::vector<cv::Point2f>> plain_markers;
    std::vector<std::shared_ptr<std::vector<std::shared_ptr<cv::Point2f>>>> markers = *(imgMarkers->markers);
    plain_markers.reserve(markers.size());
    for (std::shared_ptr<std::vector<std::shared_ptr<cv::Point2f>>> &points : markers) {
        std::vector<cv::Point2f> ps;
        for (const auto &point : *points) {
            ps.push_back(*point);
        }
        plain_markers.push_back(ps);
    }
    cv::aruco::drawDetectedMarkers(image, plain_markers, (*imgMarkers->ids), cv::Scalar(0, 255, 0));
    Logger::Instance()->Info("[Aruco]: Wrote marked up baseline for {} to {}", name, markedFile);
    cv::imwrite(markedFile, image);

    if (imgMarkers->markers->size() != expectedMarkerCount) {
        throw std::runtime_error( std::to_string((int)((100.0 * imgMarkers->markers->size()))/expectedMarkerCount) + "% expected markers matched in " + srcImgFile);
    }

    this->baselineCandidates->insert(std::make_pair(name, imgMarkers));
}

std::shared_ptr<HomographyResult> ArucoTransforms::to_homog_result(cv::Mat &targetImage, std::string name, std::shared_ptr<BestMarkers> bestMatches, std::string debugDrawDir) {
    // Trim the worst matching markers until at most maxCandidateMarkerMatches remain
    int maxMatchesSeen = bestMatches->matches->markers->size();
    int newSize = maxMatchesSeen > maxCandidateMarkerMatches ? maxCandidateMarkerMatches : maxMatchesSeen;
    int trimCount = maxMatchesSeen > maxCandidateMarkerMatches ? (maxMatchesSeen - maxCandidateMarkerMatches) : 0;
    auto truncatedMarkers = std::make_shared<std::vector<std::shared_ptr<std::vector<std::shared_ptr<cv::Point2f>>>>>();
    auto truncatedIds = std::make_shared<std::vector<int>>();
    for (int i = 0; i < newSize; i++) {
        truncatedMarkers->push_back(bestMatches->matches->markers->at(i));
        truncatedIds->push_back(bestMatches->matches->ids->at(i));
    }
    auto truncatedBest = std::make_shared<ImageMarkers>(truncatedMarkers, truncatedIds);
    // For each marker / set of 4 points, order the indexes in both candidate and target arrays
    // to best match up... TODO
    
    // Flatten out all baseline marker points whose ID also exists in the target image
    auto trimmedCandidatePoints = std::make_shared<std::vector<cv::Point2f>>();
    auto trimmedIds = std::make_shared<std::vector<int>>();
    auto trimmedMarkers = std::make_shared<std::vector<std::shared_ptr<std::vector<std::shared_ptr<cv::Point2f>>>>>();
    auto baseline = this->baselineCandidates->at(name);
    for (int i = 0; i < truncatedBest->ids->size(); i++) {
        auto id = (*truncatedBest->ids)[i];
        int candidateIdIndex = 0;
        for (; id != (*baseline->ids)[candidateIdIndex]; candidateIdIndex++) {
        }
      
        auto candidateMarkerPoints = (*baseline->markers)[candidateIdIndex];
        trimmedMarkers->push_back(candidateMarkerPoints);
        trimmedIds->push_back(id);
        trimmedCandidatePoints->push_back(*(*candidateMarkerPoints)[0]);
        trimmedCandidatePoints->push_back(*(*candidateMarkerPoints)[1]);
        trimmedCandidatePoints->push_back(*(*candidateMarkerPoints)[2]);
        trimmedCandidatePoints->push_back(*(*candidateMarkerPoints)[3]);
    }
    // Flatten out all the best markers into a point array
    std::vector<cv::Point2f> bestMatchPoints;
    for (int i = 0; i < truncatedBest->markers->size(); i++) {
        auto corners = (*truncatedBest->markers)[i];
        bestMatchPoints.push_back(*(*corners)[0]);
        bestMatchPoints.push_back(*(*corners)[1]);
        bestMatchPoints.push_back(*(*corners)[2]);
        bestMatchPoints.push_back(*(*corners)[3]);
    }
    cv::Mat a = cv::Mat(*trimmedCandidatePoints);
    cv::Mat b = cv::Mat(bestMatchPoints);
    
    auto homog = cv::findHomography(b, a, cv::RANSAC);
    if (debugDrawDir != "") {
        cv::Mat copy = targetImage.clone();
        // Sadly need to un-shared_ptr-ify the array for openCV to be happy
        std::vector<std::vector<cv::Point2f>> unsharedBestMarkers;
        std::vector<std::vector<cv::Point2f>> unsharedBaselineMarkers;
        for (int i = 0; i < truncatedBest->markers->size(); i++) {
            std::vector<std::shared_ptr<cv::Point2f>> points = *((*truncatedBest->markers)[i]);
            std::vector<cv::Point2f> flat;
            for (const auto point : points) {
                flat.push_back(*point);
            }
            unsharedBestMarkers.push_back(flat);
        }
        for (int i = 0; i < baseline->markers->size(); i++) {
            std::vector<std::shared_ptr<cv::Point2f>> points = *((*baseline->markers)[i]);
            std::vector<cv::Point2f> flat;
            for (const auto point : points) {
                flat.push_back(*point);
            }
            unsharedBaselineMarkers.push_back(flat);
            //std::vector<std::shared_ptr<cv::Point2f>> el = *((*baseline->markers)[i]);
            //unsharedBaselineMarkers.push_back(*(*baseline->markers)[i]);
        }
        // Draw best matching aruco boxes and ideal baseline aruco markers
        cv::aruco::drawDetectedMarkers(copy, unsharedBestMarkers, *(truncatedBest->ids), cv::Scalar(0, 255, 0));
        cv::aruco::drawDetectedMarkers(copy, unsharedBaselineMarkers, *(baseline->ids), cv::Scalar(0, 0, 255));

        // Draw lines connecting ideal baseline boxes with best marker matches
        for (int i = 0; i < trimmedCandidatePoints->size(); i++) {
            cv::line(copy, (*trimmedCandidatePoints)[i], bestMatchPoints[i], cv::Scalar(255, 255, 0), 3);
        }

        std::string image_path = debugDrawDir + "markers_selected_" + name + ".jpeg";
        cv::imwrite(image_path, copy);
    }
    auto trimmedCandidates = std::make_shared<ImageMarkers>(trimmedMarkers, trimmedIds);
    auto new_best = std::make_shared<BestMarkers>(truncatedBest, bestMatches->distance_score);
    return std::make_shared<HomographyResult>(new_best, trimmedCandidates, homog, name, maxMatchesSeen);
}

struct less_than_homog
{
    inline bool operator() (const std::shared_ptr<HomographyResult>& struct1, const std::shared_ptr<HomographyResult>& struct2)
    {
        return struct1->imgMarkers->distance_score < struct2->imgMarkers->distance_score;
    }
};

std::shared_ptr<std::vector<std::shared_ptr<HomographyResult>>> ArucoTransforms::findImgHomographyFromMarkers(cv::Mat targetImage, std::string debugDrawDir) {
    auto results = std::make_shared<std::vector<std::shared_ptr<HomographyResult>>>();
    // Iterate over all baselineCandidates to findBestMarkers
    Logger::Instance()->Info("[Aruco]: baselineCandidates size = {}", this->baselineCandidates->size());
    for (auto it = this->baselineCandidates->begin(); it != this->baselineCandidates->end(); ++it) {
        auto marks = detectArucoMarkers(targetImage, it->first);
        auto curSize = (int)marks->markers->size();

        if (curSize < minCandidateMarkerMatches) {
            Logger::Instance()->Info("[Aruco]: Insufficient markers seen for {} ()", it->first, curSize);
            continue;
        }
        Logger::Instance()->Info("[Aruco]: find best in {}", it->first);
        auto bestMatches = findBestMarkers(targetImage, it->second, marks);
        results->push_back(to_homog_result(targetImage, it->first, bestMatches, debugDrawDir));
        //Logger::Instance()->Info("[Aruco]: Eval candidate: {} scored {}", it->first, best->distance_score);
    }
    if (results->size() == 0) {
        Logger::Instance()->Error("[Aruco]: No good Aruco baseline candidate!");
    }

    // Sort results by score ascending order
    std::sort(results->begin(), results->end(), less_than_homog());
    Logger::Instance()->Error("[Aruco]: winner {}", (*results)[0]->bestCandidateName);
    return results;
}

cv::Mat ArucoTransforms::applyHomographyToImg(cv::Mat img, std::shared_ptr<HomographyResult> homography) {
    cv::Mat warped;
    cv::warpPerspective(img, warped, homography->transformMatrix, img.size());
    return warped;
}
