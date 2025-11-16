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
 * 
 * 1. tes tdiff optical flow algorithms
 * 
 * 2. also output stuff to graphs, plot slip angle over time
 * get statistics on how long it takes to process each frame, stdev, mean, median, etc.
 */

#include <iostream>
#include <fstream>
#include <chrono>
#include <opencv2/core.hpp>

#include "Slippery/Slippery.hpp"

using namespace Slip; // usage acceptable b/c out namespace

int main(int argc, char **argv) {

    const std::string about =
        "This code performs slip angle calculation using Lucas Kanade Optical Flow and feature detection.\n"
        "Test videos at Box/BFR/MK11/11 Software/slip_angle_tests\n"
        "Based primarily on https://files.slack.com/files-pri/T01827RH821-F09253879JT/johnson2019.pdf";
    const std::string keys =
        "{ h help |      | print this help message }"
        "{ @video_in | original.mp4 | path to video input (mp4, mov should probably work. avi should definitely work) }"
        "{ @video_out | output.avi | path to video output (avi format) }"
        "{ stats_file | | path to optional stats output (csv format) }";
    cv::CommandLineParser parser(argc, argv, keys);
    parser.about(about);
    if (parser.has("help"))
    {
        parser.printMessage();
        return 0;
    }
    std::string filename_in = cv::samples::findFile(parser.get<std::string>("@video_in"));
    std::string filename_out = parser.get<std::string>("@video_out");
    bool stats_enabled = parser.has("stats_file");
    std::string filename_stats;

    if (stats_enabled) {
        std::cout << parser.get<std::string>("stats_file") << std::endl;
        filename_stats = parser.get<std::string>("stats_file");
    }
    if (!parser.check()) {
        parser.printErrors();
        return 0;
    }
    cv::VideoCapture capture(filename_in);
    if (!capture.isOpened()) {
        std::cerr << "Unable to open file!" << std::endl;
        return 0;
    }


    auto optional_flow = OpticalFlow::configure(filename_in, filename_out);

    if (!optional_flow) {
        std::cerr << "Flow failed to initialize\n" << std::endl;
        return 1;
    }

    OpticalFlow flow = std::move(*optional_flow);

    bool is_done = false;

    while (!is_done) {
        is_done = flow.step();
    } 

    flow.writer.release();

    std::cout << filename_stats << std::endl;
    if (stats_enabled) {
        std::ofstream stats_out(filename_stats);
        stats_out << "Frame, Angle (deg), Time (s)" << std::endl;

        int FRAME_COUNT = capture.get(cv::CAP_PROP_FRAME_COUNT);

        for (size_t i = 0; i < FRAME_COUNT; i++) {
            stats_out << i << "," << flow.samples.at(i).angle << "," << flow.samples.at(i).time << std::endl;
        }
        stats_out.close();
    }

    return 0;
}
