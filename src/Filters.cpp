/*
* Filters.cpp
* Implements various image filters using OpenCV.
*/

#include "../hpp/Filters.hpp"
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
using namespace std;
using namespace cv;

Mat filterPixelate(Mat frame) {
    Mat tinyFrame, bigFrame;
    int pixelation = 5;
    resize(frame, tinyFrame, Size(frame.cols / pixelation, frame.rows / pixelation), 0, 0, INTER_NEAREST);
    resize(tinyFrame, bigFrame, Size(frame.cols, frame.rows), 0, 0, INTER_NEAREST);
    return bigFrame;
}

Mat filterCanny(Mat frame) {
    Mat temp;
    cvtColor(frame, temp, COLOR_BGR2GRAY);
    Canny(temp, temp, 60, 180);
    cvtColor(temp, temp, COLOR_GRAY2BGR);
    return temp;
}

Mat filterSinCity(Mat frame) {
    int divisor = 256 / 4;
    frame = (frame / divisor) * divisor;

    Mat channels[3];
    split(frame, channels);
    Mat blue = channels[0], green = channels[1], red = channels[2];
    Mat redMask = (red > 100) & (green < 50) & (blue < 50);

    cvtColor(frame, frame, COLOR_BGR2GRAY);
    frame = (frame > 90) * 255;
    cvtColor(frame, frame, COLOR_GRAY2BGR);
    frame.setTo(Scalar(0, 0, 255), redMask);
    return frame;
}

Mat filterSinCity2(Mat frame) {
    Mat canny = filterCanny(frame);

    Mat saturatedRed;
    extractChannel(frame, saturatedRed, 2);
    cvtColor(frame, frame, COLOR_BGR2GRAY);
    cvtColor(frame, frame, COLOR_GRAY2BGR);
    insertChannel(saturatedRed, frame, 2);

    Mat edges;
    extractChannel(canny, edges, 0);
    frame.setTo(Scalar(0, 0, 0), edges);
    return frame;
}

void applyCVFilter(int& mode, Mat& frame) {
    if (mode == 0) {
        cvtColor(frame, frame, COLOR_BGR2GRAY);
        cvtColor(frame, frame, COLOR_GRAY2BGR);
    }
    else if (mode == 1) {
        frame = filterPixelate(frame);
    }
    else if (mode == 2) {
        frame = filterSinCity(frame);
    }
    else if (mode == 3) {
        medianBlur(frame, frame, 15);
    }
}
