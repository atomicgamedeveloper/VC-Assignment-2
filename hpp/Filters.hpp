#ifndef FILTERS_HPP
#define FILTERS_HPP

#include <opencv2/opencv.hpp>
using namespace cv;

Mat filterPixelate(Mat frame);
Mat filterCanny(Mat frame);
Mat filterSinCity(Mat frame);
Mat filterSinCity2(Mat frame);
void applyCVFilter(int& mode, Mat& frame);

#endif
