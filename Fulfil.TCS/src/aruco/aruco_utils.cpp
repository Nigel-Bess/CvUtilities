#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/core.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>

#include <algorithm>
#include <limits.h>
#include "aruco/aruco_utils.h"
#include <numeric>
#include "Fulfil.CPPUtils/logging.h"
#include <filesystem>

using fulfil::utils::aruco::ArucoTransforms;
using fulfil::utils::aruco::ImageMarkers;
using fulfil::utils::aruco::HomographyResult;
using fulfil::utils::aruco::BestMarkers;
using fulfil::utils::Logger;

const auto EMPTY_POINT_ARRAY = std::make_shared<std::vector<std::shared_ptr<std::vector<cv::Point2f>>>>();
const auto EMPTY_INT_ARRAY = std::make_shared<std::vector<int>>();
const auto EMPTY_IMG_MARKERS = std::make_shared<ImageMarkers>(ImageMarkers{EMPTY_POINT_ARRAY, EMPTY_INT_ARRAY});

ArucoTransforms::ArucoTransforms(
    cv::Ptr<cv::aruco::Dictionary> dict,
    std::shared_ptr<cv::aruco::DetectorParameters> params,
    float maxMarkerDisplacementDist,
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
        auto corners = std::make_shared<std::vector<std::vector<cv::Point2f>>>();
        auto ids = std::make_shared<std::vector<int>>();
        std::vector<std::vector<cv::Point2f>> rejected;
        cv::aruco::detectMarkers(img, this->dict, *corners, *ids, this->params, rejected);
        if (rejected.size() > 0) {
            Logger::Instance()->Info("Rejected {} markers, accepted {}", rejected.size(), corners->size());
        }

        auto sharedCorners = std::make_shared<std::vector<std::shared_ptr<std::vector<cv::Point2f>>>>();
        for (int i = 0; i < corners->size(); i++) {
            sharedCorners->push_back(std::make_shared<std::vector<cv::Point2f>>());
            for (int j = 0; j < 4; j++) {
                (*sharedCorners)[i]->push_back((*corners)[i][j]);
            }
        }
        return std::make_shared<ImageMarkers>(ImageMarkers{sharedCorners, ids});
    }
    catch (const std::exception &e)
    {
        Logger::Instance()->Info("Error in drawDetectedMarkers! ");
    }
    return EMPTY_IMG_MARKERS;
}

float ArucoTransforms::calculateMarkerDistSum(std::shared_ptr<ImageMarkers> candidate, std::shared_ptr<std::vector<cv::Point2f>> markerCorners, int markerId) {
    auto ids = candidate->ids;
    int candidateMarkerIndex = 0;
    for (; markerId != (*ids)[candidateMarkerIndex]; candidateMarkerIndex++) {}
    auto candidateCorners = (*candidate->markers)[candidateMarkerIndex];

    float distSum = INT_MAX;
    for (int i = 0; i < 4; i++) {
        auto dist = distance((*candidateCorners)[i], (*markerCorners)[i]);
        distSum = dist < distSum ? dist : distSum;
    }
    return distSum;
}

std::shared_ptr<BestMarkers> ArucoTransforms::findBestMarkers(std::shared_ptr<ImageMarkers> candidate, std::shared_ptr<ImageMarkers> instance) {
    auto cids = candidate->ids;
    // First find all best tag matches and key them by marker IDs
    std::map<int, std::shared_ptr<std::vector<cv::Point2f>>> id_to_best_fit;
    std::map<int, int> id_to_dist_sum;

    int mCount = instance->markers->size();
    for (int i = 0; i < mCount; i++) {
        auto corners = (*instance->markers)[i];
        auto id = (*instance->ids)[i];
        // Ignore IDs not shared with candidate
        if (std::find(cids->begin(), cids->end(), id) != cids->end()) {
            bool writeThisID = false;
            // If ID already seen, take the one with the smallest candidate diff
            if (id_to_best_fit.find(id) != id_to_best_fit.end()) {
                float last_dist = 99999999;
                if (id_to_dist_sum.find(id) != id_to_dist_sum.end()) {
                    last_dist = id_to_dist_sum.at(id);
                }
                auto cur_dist = calculateMarkerDistSum(candidate, corners, id);
                if (cur_dist < last_dist) {
                    writeThisID = true;
                }
            } else {
                writeThisID = true;
            }
            if (writeThisID) {
                id_to_dist_sum.insert(std::make_pair(id, calculateMarkerDistSum(candidate, corners, id)));
                id_to_best_fit.insert(std::make_pair(id, corners));
            }
        }
    }

    // Convert map to flat array and sort by shortest diff distances
    auto markers = std::make_shared<std::vector<std::shared_ptr<std::vector<cv::Point2f>>>>();
    auto ids = std::make_shared<std::vector<int>>();
    std::vector<float> distances;
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
                (*markers)[i].swap((*markers)[j]);
                std::swap((*ids)[i], (*ids)[j]);
                std::swap(distances[i], distances[j]);
            }
        }
    }

    // Distance sum is the LITERAL sum of distances between candidate marker corners and
    // current img marker corners except candidate markers missing from cur image are penalized
    // by the worst-case 4 corner marker diff so that candidates with less matching markers are discouraged
    int longestDistanceSum = *std::max_element(distances.begin(), distances.end());
    int distanceSum = std::accumulate(distances.begin(), distances.end(), 0);
    int missingCount = candidate->markers->size() - ids->size();
    float missingIdsPenalty = missingCount * longestDistanceSum;
    distanceSum += missingIdsPenalty;
    return std::make_shared<BestMarkers>(BestMarkers{std::make_shared<ImageMarkers>(ImageMarkers{markers, ids}), distanceSum});
}

void ArucoTransforms::loadBaselineImageAsCandidate(std::string srcImgFile, std::string name, int expectedMarkerCount) {
    if (!std::filesystem::exists(srcImgFile)) {
        throw std::runtime_error("Failed to find baseline image " + srcImgFile);
    }

    cv::Mat image = cv::imread(srcImgFile, cv::IMREAD_COLOR);
    auto imgMarkers = detectArucoMarkers(image, name);
    if (imgMarkers->markers->size() != expectedMarkerCount) {
        throw std::runtime_error( std::to_string((int)((100.0 * imgMarkers->markers->size()))/expectedMarkerCount) + "% expected markers matched in " + srcImgFile);
    }

    // Uncomment to view all loaded basline images for debugging purposes in data/test
    /*auto markedFile = "Fulfil.TCS/data/test/" + name + "_marked_" + std::to_string(expectedMarkerCount) + ".jpeg";
    cv::aruco::drawDetectedMarkers(image, imgMarkers->markers, imgMarkers->ids, cv::Scalar(0, 255, 0));
    Logger::Instance()->Info("Wrote marked up baseline for {} to {}", name, markedFile);
    cv::imwrite(markedFile, image);*/

    this->baselineCandidates->insert(std::make_pair(name, imgMarkers));
}

std::shared_ptr<HomographyResult> ArucoTransforms::findImgHomographyFromMarkers(cv::Mat targetImage, std::string debugDrawDir) {
    // Iterate over all baselineCandidates to findBestMarkers
    float smallest_diff = INT_MAX;
    std::string winningCandidateName = "";
    std::shared_ptr<ImageMarkers> bestMatches = EMPTY_IMG_MARKERS;
    int maxMatchesSeen = 0;

    for (auto it = this->baselineCandidates->begin(); it != this->baselineCandidates->end(); ++it) {
        auto marks = detectArucoMarkers(targetImage, it->first);
        auto curSize = (int)marks->markers->size();
        maxMatchesSeen = std::max(maxMatchesSeen, curSize);
        if (curSize < minCandidateMarkerMatches) {
            continue;
        }
        auto best = findBestMarkers(it->second, marks);
        if (best->distanceSum < smallest_diff) {
            smallest_diff = best->distanceSum;
            winningCandidateName = it->first;
            bestMatches = best->matches;
        }
    }

    if (smallest_diff == INT_MAX) {
        return std::make_shared<HomographyResult>(HomographyResult{EMPTY_IMG_MARKERS, EMPTY_IMG_MARKERS, cv::Mat(), "", maxMatchesSeen});
    }

    // Now trim the worst matching markers until at most maxCandidateMarkerMatches remain
    maxMatchesSeen = bestMatches->markers->size();
    int newSize = maxMatchesSeen > maxCandidateMarkerMatches ? maxCandidateMarkerMatches : maxMatchesSeen;
    int trimCount = maxMatchesSeen > maxCandidateMarkerMatches ? (maxMatchesSeen - maxCandidateMarkerMatches) : 0;

    auto truncatedMarkers = std::make_shared<std::vector<std::shared_ptr<std::vector<cv::Point2f>>>>();
    auto truncatedIds = std::make_shared<std::vector<int>>();

    for (int i = 0; i < newSize; i++) {
        truncatedMarkers->push_back(bestMatches->markers->at(i));
        truncatedIds->push_back(bestMatches->ids->at(i));
    }
    auto truncatedBest = std::make_shared<ImageMarkers>(ImageMarkers{truncatedMarkers, truncatedIds});
    // For each marker / set of 4 points, order the indexes in both candidate and target arrays
    // to best match up... TODO
    
    // Flatten out all baseline marker points whose ID also exists in the target image
    auto trimmedCandidatePoints = std::make_shared<std::vector<cv::Point2f>>();
    auto trimmedIds = std::make_shared<std::vector<int>>();
    auto trimmedMarkers = std::make_shared<std::vector<std::shared_ptr<std::vector<cv::Point2f>>>>();

    auto baseline = this->baselineCandidates->at(winningCandidateName);
    for (int i = 0; i < truncatedBest->ids->size(); i++) {
        auto id = (*truncatedBest->ids)[i];
        int candidateIdIndex = 0;
        for (; id != (*baseline->ids)[candidateIdIndex]; candidateIdIndex++) {
        }

        auto candidateMarkerPoints = (*baseline->markers)[candidateIdIndex];
        trimmedMarkers->push_back(candidateMarkerPoints);
        trimmedIds->push_back(id);
        trimmedCandidatePoints->push_back((*candidateMarkerPoints)[0]);
        trimmedCandidatePoints->push_back((*candidateMarkerPoints)[1]);
        trimmedCandidatePoints->push_back((*candidateMarkerPoints)[2]);
        trimmedCandidatePoints->push_back((*candidateMarkerPoints)[3]);
    }

    // Flatten out all the best markers into a point array
    std::vector<cv::Point2f> bestMatchPoints;
    for (int i = 0; i < truncatedBest->markers->size(); i++) {
        auto corners = (*truncatedBest->markers)[i];
        bestMatchPoints.push_back((*corners)[0]);
        bestMatchPoints.push_back((*corners)[1]);
        bestMatchPoints.push_back((*corners)[2]);
        bestMatchPoints.push_back((*corners)[3]);
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
            unsharedBestMarkers.push_back(*(*truncatedBest->markers)[i]);
        }
        for (int i = 0; i < baseline->markers->size(); i++) {
            unsharedBaselineMarkers.push_back(*(*baseline->markers)[i]);
        }
        // Draw best matching aruco boxes and ideal baseline aruco markers
        cv::aruco::drawDetectedMarkers(copy, unsharedBestMarkers, *(truncatedBest->ids), cv::Scalar(0, 255, 0));
        cv::aruco::drawDetectedMarkers(copy, unsharedBaselineMarkers, *(baseline->ids), cv::Scalar(0, 0, 255));

        // Draw lines connecting ideal baseline boxes with best marker matches
        for (int i = 0; i < trimmedCandidatePoints->size(); i++) {
            cv::line(copy, (*trimmedCandidatePoints)[i], bestMatchPoints[i], cv::Scalar(255, 255, 0), 3);
        }

        std::string image_path = debugDrawDir + "markers_selected_" + winningCandidateName + ".jpeg";
        cv::imwrite(image_path, copy);
    }

    auto trimmedCandidates = std::make_shared<ImageMarkers>(ImageMarkers{trimmedMarkers, trimmedIds});
    return std::make_shared<HomographyResult>(HomographyResult{truncatedBest, trimmedCandidates, homog, winningCandidateName, maxMatchesSeen});
}

cv::Mat ArucoTransforms::applyHomographyToImg(cv::Mat img, std::shared_ptr<HomographyResult> homography) {
    cv::Mat warped;
    cv::warpPerspective(img, warped, homography->transformMatrix, img.size());
    return warped;
}
