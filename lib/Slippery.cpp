#include "Slippery/Slippery.hpp"

namespace Slip {

    std::optional<OpticalFlow> OpticalFlow::configure(std::string filename_in, std::string filename_out) {
        cv::VideoCapture capture(filename_in);
        if (!capture.isOpened()) {
            std::cerr << "Unable to open file!" << std::endl;
            return {};
        }

        cv::Mat prev_frame, prev_gray;

        capture >> prev_frame;
        if (prev_frame.empty()) {
            return {};
        }
        cv::cvtColor(prev_frame, prev_gray, cv::COLOR_BGR2GRAY);

        double FPS = capture.get(cv::CAP_PROP_FPS);
        cv::VideoWriter writer(filename_out, cv::VideoWriter::fourcc('M','J','P','G'), FPS, prev_frame.size(), true);

        return OpticalFlow(prev_frame, prev_gray, capture, writer);
    }

    bool OpticalFlow::step() {
        SlipAngleSample sample;

        auto frame_start = std::chrono::system_clock::now();
        sample.time = std::chrono::duration<double>(frame_start - start).count();

        cv::Mat frame, gray;
        capture >> frame;

        if (frame.empty()) {
            return true; // DONE?
        }

        cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);

        // feature detection, uses some eigenvalue thing
        std::vector<cv::Point2f> prev_points;
        cv::goodFeaturesToTrack(prev_gray, prev_points, 300, 0.3, 7, cv::Mat(), 7, false, 0.04);
        cv::TermCriteria crit = cv::TermCriteria(cv::TermCriteria::COUNT + cv::TermCriteria::EPS, 20, 0.03);
        
        std::vector<cv::Point2f> new_points;
        std::vector<uchar> status;
        std::vector<float> err;

        cv::calcOpticalFlowPyrLK(prev_gray, gray,
                                prev_points, new_points,
                                status, err,
                                cv::Size(15,15), 3, crit);


        // array representing dx, dy for each point
        std::vector<double> slip_angles;
        slip_angles.reserve(new_points.size());
        
        for (size_t i = 0; i < new_points.size(); i++) {
            cv::Point2f diff = (new_points[i] - prev_points[i]) * int(status[i]);
            cv::Point2f d = diff;
            double slip_angle = atan2(d.y, d.x) * 180.0 / CV_PI;
            slip_angles.push_back(slip_angle);
        }
        std::sort(slip_angles.begin(), slip_angles.end());
        // BOOM!
        double median_slip_angle = slip_angles[slip_angles.size() / 2];
        sample.angle = median_slip_angle;

        samples.push_back(sample);

        //draw
        for (size_t i = 0; i < prev_points.size(); i++) {
            if (!status[i]) {
                continue;
            }

            cv::Point2f p0 = prev_points[i];
            cv::Point2f p1 = new_points[i];

            cv::Scalar col = cv::Scalar(0, 255, 0);

            cv::line(frame, p0, p1, col, 2);

            cv::circle(frame, p1, 4, col, -1);
        } // display median slip angle on frame

        cv::putText(frame,
            cv::format("%.2f", median_slip_angle),
            cv::Point(10, frame.rows / 2),
            cv::FONT_HERSHEY_DUPLEX,
            3.0,
            CV_RGB(0, 0, 0),
            2);

        writer.write(frame);

        prev_gray = gray.clone();
        frame_num++;        
        if (frame_num % 100 == 0) {
            int frame_count = capture.get(cv::CAP_PROP_FRAME_COUNT);

            std::cout << frame_num << "/" << frame_count << " frames processed" << std::endl;
        }

        return false;
    }

} // Slip