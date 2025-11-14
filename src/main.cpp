/**
 * CURRENT CODE STATUS - VINAY
 * Slip angle algorithm pretty much works as described in https://files.slack.com/files-pri/T01827RH821-F09253879JT/johnson2019.pdf
 * Key differences:
 * * No distortion removal
 * * Instead of RANSAC or something sophisticated for outliers, I just take the median of slip angles for each feature
 * * Just displays to user, no saving
 * 
 * What needs to be done:
 * Slip angle doesn't work with current camera setup. The paper has the camera pointed directly down,
 * whereas here it's angled up.
 * -> This means depth must be taken into account, Owen says we should do a linear projection.
 * 
 * If we want to change the camera setup, we should point it directly downwards and angle it so the right/left direction
 * is aligned with the direction the car is pointing.
 * 
 * Known issues:
 * whatever i said above
 * feature tracking doesn't work sometimes, tune parameters or something
 * need to preprocess all the videos
 */

#include <iostream>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/video.hpp>
using namespace cv;
using namespace std;

int main(int argc, char **argv)
{
    const string about =
        "This sample demonstrates Lucas-Kanade Optical Flow calculation.\n"
        "The example file can be downloaded from:\n"
        "  https://www.bogotobogo.com/python/OpenCV_Python/images/mean_shift_tracking/slow_traffic_small.mp4";
    const string keys =
        "{ h help |      | print this help message }"
        "{ @image | vtest.avi | path to image file }";
    CommandLineParser parser(argc, argv, keys);
    parser.about(about);
    if (parser.has("help"))
    {
        parser.printMessage();
        return 0;
    }
    string filename = samples::findFile(parser.get<string>("@image"));
    if (!parser.check())
    {
        parser.printErrors();
        return 0;
    }
    VideoCapture capture(filename);
    if (!capture.isOpened()) {
        cerr << "Unable to open file!" << endl;
        return 0;
    }

    Mat prevFrame, prevGray;
    capture >> prevFrame;
    if (prevFrame.empty()) return 0;
    cvtColor(prevFrame, prevGray, COLOR_BGR2GRAY);

    VideoWriter writer("../output.mp4", VideoWriter::fourcc('M','P','4','V'), 30, prevFrame.size());
    int frameNum = 0;
    while (true)
    {
        Mat frame, gray;
        capture >> frame;
        if (frame.empty()) break;
        // if (frameNum < 1000) continue;

        cvtColor(frame, gray, COLOR_BGR2GRAY);
        
        // idk why I can't put this outside the loop
        // masking out the top half, too much interference
        // TODO: remove this once camera is properly mounted
        bool MASKED = false;
        Mat bottom_half_mask = Mat::zeros(frame.size(), CV_8UC1);
        for (int i = bottom_half_mask.rows/2; i<bottom_half_mask.rows; i++)
            for (int j = 0; j<bottom_half_mask.cols; j++)
                bottom_half_mask.at<Vec3b>(i, j) = Vec3b(255, 255, 255);

        // feature detection
        // _currPts unused
        vector<Point2f> prevPts, _currPts;
        goodFeaturesToTrack(prevGray, prevPts, 300, 0.3, 7, MASKED ? bottom_half_mask : Mat(), 7, false, 0.04);
        // goodFeaturesToTrack(gray,     _currPts, 300, MASKED ? 0.18 : 0.3, 7, MASKED ? bottom_half_mask : Mat(), 7, false, 0.04);

        // optical flow w Lucas Kanade into new_points
        vector<Point2f> new_points;
        vector<uchar> status;
        vector<float> err;

        TermCriteria crit = TermCriteria(TermCriteria::COUNT + TermCriteria::EPS, 20, 0.03);
        calcOpticalFlowPyrLK(prevGray, gray,
                             prevPts, new_points,
                             status, err,
                             Size(15,15), 3, crit);


        // array representing dx, dy for each point
        vector<Point2f> diffs;        
        for (size_t i = 0; i < new_points.size(); i++)
        {
            Point2f diff = (new_points[i] - prevPts[i]) * int(status[i]);
            // cout << diff.x << ' ' << diff.y << ' ';
            diffs.push_back(diff);
        }
        
        vector<float> slip_angles;
        for (size_t i = 0; i < diffs.size(); i++)
        {
            Point2f d = diffs[i];
            float slip_angle = atan2(d.y, d.x) * 180.0 / CV_PI;
            slip_angles.push_back(slip_angle);
        }
        sort(slip_angles.begin(), slip_angles.end());
        float median_slip_angle = slip_angles[slip_angles.size() / 2];
        // draw and display stuff
        for (size_t i = 0; i < prevPts.size(); i++)
        {
            if (!status[i]) continue;

            Point2f p0 = prevPts[i];
            Point2f p1 = new_points[i];

            Scalar col = Scalar(0, 255, 0);

            line(frame, p0, p1, col, 2);

            circle(frame, p1, 4, col, -1);
        }
        putText(frame, //target image
            format("%.2f", median_slip_angle), //text
            cv::Point(10, frame.rows / 2), //top-left position
            cv::FONT_HERSHEY_DUPLEX,
            3.0,
            CV_RGB(0, 0, 0), //font color
            2);

        namedWindow("Display frame", WINDOW_NORMAL); 
        resizeWindow("Display frame", 1080/2, 1920/2); 
        imshow("Display frame", frame);

        // so we can step by frame
        // int key = waitKey(16);
        // if (key == 'q' || key == 27) break;

        prevGray = gray.clone();
        writer.write(frame);
        frameNum++;
        cout<<frameNum<<" frames processed\n"<<endl;
    }

    return 0;
}
