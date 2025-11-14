/**
 * CURRENT CODE STATUS - VINAY
 * Slip angle algorithm pretty much works as described in https://files.slack.com/files-pri/T01827RH821-F09253879JT/johnson2019.pdf
 * Key differences:
 * * No distortion removal
 * * Instead of RANSAC or something sophisticated for outliers, I just take the median of slip angles for each feature
 * * Saves to output video with drawn features and slip angle
 * 
 * What needs to be done:
 * Slip angle doesn't work with current camera setup. The paper has the camera pointed directly down,
 * whereas here it's angled up.
 * -> This means depth must be taken into account, Owen says we should do a linear projection.
 * 
 * If we want to change the camera setup, we should point it directly downwards and angle it so the right/left direction
 * is aligned with the direction the car is pointing.
 * 
 * Other issues:
 * feature tracking doesn't work sometimes, tune parameters or something
 * fix output to be in mp4 format
 * Random !filename_pattern.empty() error, doesn't seem to do anything though
 * Videos should to be put in some kind of preprocessing pipeline w/ffmpeg
 */

#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/video.hpp>

int main(int argc, char **argv)
{
    const std::string about =
        "This code performs slip angle calculation using Lucas Kanade Optical Flow and feature detection.\n"
        "Test videos at Box/BFR/MK11/11 Software/slip_angle_tests\n"
        "Based primarily on https://files.slack.com/files-pri/T01827RH821-F09253879JT/johnson2019.pdf";
    const std::string keys =
        "{ h help |      | print this help message }"
        "{ @video_in | original.mp4 | path to video input (mp4, mov should probably work. avi should definitely work) }"
        "{ @video_out | output.avi | path to video output (avi format) }";
    cv::CommandLineParser parser(argc, argv, keys);
    parser.about(about);
    if (parser.has("help"))
    {
        parser.printMessage();
        return 0;
    }
    std::string filename_in = cv::samples::findFile(parser.get<std::string>("@video_in"));
    std::string filename_out = parser.get<std::string>("@video_out");
    if (!parser.check())
    {
        parser.printErrors();
        return 0;
    }
    cv::VideoCapture capture(filename_in);
    if (!capture.isOpened()) {
        std::cerr << "Unable to open file!" << std::endl;
        return 0;
    }

    cv::Mat prevFrame, prevGray;
    capture >> prevFrame;
    if (prevFrame.empty()) return 0;
    cv::cvtColor(prevFrame, prevGray, cv::COLOR_BGR2GRAY);

    double FPS = capture.get(cv::CAP_PROP_FPS);
    int FRAME_COUNT = capture.get(cv::CAP_PROP_FRAME_COUNT);
    // codec can be changed, idrc
    cv::VideoWriter writer(filename_out, cv::VideoWriter::fourcc('M','J','P','G'), FPS, prevFrame.size(), true);
    int frameNum = 0;
    
    // Option: turn this on to mask out the top half to decrease interference
    // Remove this once camera is properly mounted
    bool MASKED = false;
    cv::Mat bottom_half_mask = cv::Mat::zeros(prevFrame.size(), CV_8UC1);
    for (int i = bottom_half_mask.rows/2; i<bottom_half_mask.rows; i++)
        for (int j = 0; j<bottom_half_mask.cols; j++)
            bottom_half_mask.at<uchar>(i, j) = 255;

    while (true)
    {
        cv::Mat frame, gray;
        capture >> frame;
        if (frame.empty()) break;

        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

        // feature detection, uses some eigenvalue thing
        std::vector<cv::Point2f> prevPts;
        cv::goodFeaturesToTrack(prevGray, prevPts, 300, 0.3, 7, MASKED ? bottom_half_mask : cv::Mat(), 7, false, 0.04);

        // optical flow with Lucas Kanade into new_points
        std::vector<cv::Point2f> new_points;
        std::vector<uchar> status;
        std::vector<float> err;

        cv::TermCriteria crit = cv::TermCriteria(cv::TermCriteria::COUNT + cv::TermCriteria::EPS, 20, 0.03);
        cv::calcOpticalFlowPyrLK(prevGray, gray,
                             prevPts, new_points,
                             status, err,
                             cv::Size(15,15), 3, crit);


        // array representing dx, dy for each point
        std::vector<cv::Point2f> diffs;        
        for (size_t i = 0; i < new_points.size(); i++)
        {
            cv::Point2f diff = (new_points[i] - prevPts[i]) * int(status[i]);
            diffs.push_back(diff);
        }

        std::vector<float> slip_angles;
        for (size_t i = 0; i < diffs.size(); i++)
        {
            cv::Point2f d = diffs[i];
            float slip_angle = atan2(d.y, d.x) * 180.0 / CV_PI;
            slip_angles.push_back(slip_angle);
        }
        std::sort(slip_angles.begin(), slip_angles.end());
        // BOOM!
        float median_slip_angle = slip_angles[slip_angles.size() / 2];

        // draw and display stuff
        for (size_t i = 0; i < prevPts.size(); i++)
        {
            if (!status[i]) continue;

            cv::Point2f p0 = prevPts[i];
            cv::Point2f p1 = new_points[i];

            cv::Scalar col = cv::Scalar(0, 255, 0);

            cv::line(frame, p0, p1, col, 2);

            cv::circle(frame, p1, 4, col, -1);
        }

        // display median slip angle on frame
        cv::putText(frame,
            cv::format("%.2f", median_slip_angle),
            cv::Point(10, frame.rows / 2),
            cv::FONT_HERSHEY_DUPLEX,
            3.0,
            CV_RGB(0, 0, 0),
            2);

        // legacy frame stepping code
        // int key = waitKey(0);
        // if (key == 'q' || key == 27) break;

        prevGray = gray.clone();
        writer.write(frame);
        frameNum++;
        if (frameNum % 100 == 0) {
            std::cout << frameNum << "/" << FRAME_COUNT << " frames processed" << std::endl;
        }
    }
    writer.release();

    return 0;
}
