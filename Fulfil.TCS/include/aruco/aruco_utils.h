#ifndef FULFIL_CPPUTILS_ARUCO_UTILS_H
#define FULFIL_CPPUTILS_ARUCO_UTILS_H

#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>

namespace fulfil
{
    namespace utils
    {
        namespace aruco
        {

            /**
             * Represents a single "image" of idealized Aruco marker Ids and positions
             */
            struct ImageMarkers
            {
                std::shared_ptr<std::vector<std::shared_ptr<std::vector<cv::Point2f>>>> markers;
                std::shared_ptr<std::vector<int>> ids;
            };

            struct BestMarkers {
                /**
                 * Array of matching markers in target image
                 */
                std::shared_ptr<ImageMarkers> matches;
                /**
                 * Sum of distances between candidate and target markers
                 */
                int distanceSum;
            };

            struct HomographyResult {
                /**
                 * The raw Aruco ImageMarkers in the target image that (roughly) match to a matching baseline
                 * candidate image.
                 */
                std::shared_ptr<ImageMarkers> imgMarkers;

                /**
                 * The Aruco ImageMarkers directly from the baseline candidate's parsed
                 * markers. If using the homographically transformed image, it would
                 * roughly re-align with these markers if estimate was successful.
                 */
                std::shared_ptr<ImageMarkers> baselineMarkers;

                /**
                 * The homography transform found that can map target image points
                 * to baseline image points.  Empty matrix if no homography was found.
                 */
                cv::Mat transformMatrix;

                /**
                 * The name of the loaded baseline candidate image with Aruco markers that were most similar to
                 * target image. Set to empty string if none found.
                 */
                std::string bestCandidateName;

                /**
                 * Same as imgMarkers->size() if set, otherwise the max number of Aruco markers seen for
                 * the best (but failed) baseline candidate image.
                */
                int maxMatchesSeen;
            };


            /**
             * Handy matrix transforms based on Arcuo tags, such as finding homography matrix transforms to
             * normalize point positions as a pre-processor so new images pixel positions match baseline images.
             */
            class ArucoTransforms
            {
            private:
                /**
                 * An array of ImageMarkers, where each candidate is an array of baseline markers
                 * spread throughout an idealized annotated baseline image. Should be relatively static.
                 */
                std::map<std::string, std::shared_ptr<ImageMarkers>> *baselineCandidates;

                /**
                 * No valid marker point in realtime image should be further than this from
                 * the baseline's marker point.  Unit is in range [0, 1] as percentage of width/height
                 */
                float maxMarkerDisplacementDist;

                /**
                 * The minimum number of Baseline markers a candidate must share with the realtime image and
                 * used to find homography mappings.
                 */
                int minCandidateMarkerMatches;

                /**
                 * The maximum number of Baseline markers a candidate must share with the realtime image and
                 * used to find homography mappings.
                 */
                int maxCandidateMarkerMatches;

                /**
                 * The aruco symbol dict to use, maybe cv::aruco::DICT_4X4_50 for example
                 */
                cv::Ptr<cv::aruco::Dictionary> dict;

                /**
                 * Aruco OpenCV params to use
                 */
                std::shared_ptr<cv::aruco::DetectorParameters> params;

                /**
                 * Sum the distances between all 4 Aruco markers versus the candidate baseline marker position, assumes `id` must exist
                 * in `candidate->ids`.
                 */
                float calculateMarkerDistSum(std::shared_ptr<ImageMarkers> candidate, std::shared_ptr<std::vector<cv::Point2f>> markerCorners, int markerId);

                /**
                 * Find the best markers in an image matrix that match some ImageMarkers candidate.
                 * Sorted by markers with least distance to candidate markers of same ID.
                 */
                std::shared_ptr<BestMarkers> findBestMarkers(std::shared_ptr<ImageMarkers> candidate, std::shared_ptr<ImageMarkers> instance);

            public:
                /**
                 * ArucoTransforms Constructor
                 * @param baselineCandidates: Potential images or sets of ImageMarkers that could be used to generate a homography
                 */
                ArucoTransforms(
                    cv::Ptr<cv::aruco::Dictionary> dict,
                    std::shared_ptr<cv::aruco::DetectorParameters> params,
                    float maxMarkerDisplacementDist,
                    int minCandidateMarkerMatches,
                    int maxCandidateMarkerMatches);

                /**
                 * Highschool geometric distance between 2 2D points
                 */
                static float distance(const cv::Point2f &p1, const cv::Point2f &p2)
                {
                    return std::sqrt((p2.x - p1.x) * (p2.x - p1.x) + (p2.y - p1.y) * (p2.y - p1.y));
                }

                /**
                 * Load an image file and find the Aruco markers to record as a baseline candidate
                 * for future findImgHomographyFromMarkers calls instead of passing hand-crafted
                 * vectors into the ArucoTransforms constructor.  Throws dangerously if expectedMarkerCount
                 * is not seen in srcImgFile, so only call this around app initialization so there's no surprises.
                 */
                void loadBaselineImageAsCandidate(std::string srcImgFile, std::string name, int expectedMarkerCount);

                /*
                 * Detect the Aruco markers and drawing bounding boxes around them
                 * @param input image captured from the camera
                 * @return Marker Corner Coordinates for all the Aruco markers
                 */
                std::shared_ptr<ImageMarkers> detectArucoMarkers(cv::Mat img, std::string candidateName);

                /**
                 * The ultimate preprocessor for becoming resilient to any linear transformations (ex. camera drift)
                 * from a realtime image and make it look like the ideal baseline image! The returned homography
                 * matrix can be multipled with a point in the realtime image to map back to the baseline image's
                 * position, thus essentially undoing any rotation/translation/scale in the realtime image.
                 * May return empty vector if no valid this->baselineCandidates are found within thresholds. Note that
                 * candidate marker IDs are matched to targetImage marker IDs to compare apples-to-apples, so be sure
                 * IDs are consistent in their spacial relations across both images!
                 */
                std::shared_ptr<HomographyResult> findImgHomographyFromMarkers(cv::Mat targetImage, std::string debugDrawDir);

                /**
                 * Apply a homography matrix obtained in sibling methods to an image to rotate/scale/translate it
                 * into matching the most similar candidate in this->baselineCandidates
                 */
                cv::Mat applyHomographyToImg(cv::Mat img, std::shared_ptr<HomographyResult> homography);
            };
        }
    }
}

#endif