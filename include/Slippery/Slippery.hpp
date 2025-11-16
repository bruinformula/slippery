#pragma once 

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/video.hpp>

#include <iostream>
#include <fstream>
#include <chrono>
#include <optional>
#include <chrono>

namespace Slip {

    struct SlipAngleSample {
        double time;
        double angle;
    };

    class OpticalFlow {
    public:
        cv::Mat prev_frame, prev_gray;

        std::vector<SlipAngleSample> samples;

        cv::VideoCapture capture;
        cv::VideoWriter writer;

        int frame_num = 0;
        const std::chrono::time_point<std::chrono::system_clock> start = std::chrono::system_clock::now();

        OpticalFlow(OpticalFlow&&) = default;
        OpticalFlow& operator=(OpticalFlow&&) = delete;
        OpticalFlow(cv::Mat prev_frame, cv::Mat prev_gray, cv::VideoCapture capture, cv::VideoWriter writer) : 
            prev_frame(prev_frame),
            prev_gray(prev_gray),
            capture(capture),
            writer(writer)
        {}

        static std::optional<OpticalFlow> configure(std::string filename_in, std::string filename_out);
        
        bool step();
    };

} // Slip