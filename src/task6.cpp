#include "iostream"
#include "opencv2/imgproc.hpp"
#include "opencv2/highgui.hpp"

using namespace std;
using namespace cv;
const char* window_name = "Malik's Crazy Image Viewer 2";

using namespace std;

cv::Scalar makeVectorGrey(cv::Scalar vector) {
	cv::Scalar myGreyMultiplier(0.1, 0.6, 0.3);
	float myGrey = vector.dot(myGreyMultiplier);
	cv::Scalar result(myGrey, myGrey, myGrey);
	return result;
}

Mat TransformImage(Mat img, Mat M) {
	Mat newImg = Mat::zeros(img.size(), img.type());
	for (int row = 0; row < img.rows; row++) {
		for (int col = 0; col < img.cols; col++) {
			Vec3b pixel = img.at<cv::Vec3b>(row, col);
			Mat pos = (cv::Mat_<float>(3, 1) << col, row, 1); // Mat(cv::Vec3f(row, col, 1);
			Mat newPos = M * pos;
			int r = (int)newPos.at<float>(1, 0);
			int c = (int)newPos.at<float>(0, 0);
			if (r < img.rows && c < img.cols && r > 0 && c > 0)
			{
				newImg.at<cv::Vec3b>(r, c) = pixel;
			}
		}
	}


	printf("Transformation succeeded!");
	return newImg;
}


Mat TransformImageBackwards(Mat img, Mat M) {
	Mat invM = M.inv();
	Mat newImg = Mat::zeros(img.size(), img.type());
	for (int row = 0; row < img.rows; row++) {
		for (int col = 0; col < img.cols; col++) {
			// Destination position
			Mat pos = (cv::Mat_<float>(3, 1) << col, row, 1);

			// Source position
			Mat srcPos = invM * pos;
			int r = (int)round(srcPos.at<float>(1, 0));
			int c = (int)round(srcPos.at<float>(0, 0));

			if (r < img.rows && c < img.cols && r >= 0 && c >= 0)
			{
				// Pixel
				Vec3b srcPixel = img.at<cv::Vec3b>(r, c);
				newImg.at<cv::Vec3b>(row, col) = srcPixel;
			}
		}
	}
	printf("Transformation succeeded!");
	return newImg;
}

Mat ClaudeInverseTransformImage(Mat img, Mat M) {
	Mat invM = M.inv();
	Mat newImg = Mat::zeros(img.size(), img.type());

	// Iterate through TARGET image pixels
	for (int targetRow = 0; targetRow < newImg.rows; targetRow++) {
		for (int targetCol = 0; targetCol < newImg.cols; targetCol++) {

			// Current target position
			Mat targetPos = (cv::Mat_<float>(3, 1) << targetCol, targetRow, 1);

			// Find corresponding SOURCE position
			Mat sourcePos = invM * targetPos;
			int srcRow = (int)round(sourcePos.at<float>(1, 0));
			int srcCol = (int)round(sourcePos.at<float>(0, 0));

			// If source position is valid, copy pixel FROM source TO target
			if (srcRow >= 0 && srcRow < img.rows && srcCol >= 0 && srcCol < img.cols) {
				newImg.at<cv::Vec3b>(targetRow, targetCol) = img.at<cv::Vec3b>(srcRow, srcCol);
			}
		}
	}
	return newImg;
}

Mat InvertTransformImage(Mat img, Mat M) {
	M = M.inv();

	Mat newImg = Mat::zeros(img.size(), img.type());
	for (int row = 0; row < img.rows; row++) {
		for (int col = 0; col < img.cols; col++) {
			Vec3b pixel = img.at<cv::Vec3b>(row, col);
			Mat pos = (cv::Mat_<float>(3, 1) << col, row, 1); // Mat(cv::Vec3f(row, col, 1);
			Mat newPos = M * pos;
			int r = (int)newPos.at<float>(1, 0);
			int c = (int)newPos.at<float>(0, 0);
			if (r < img.rows && c < img.cols && r > 0 && c > 0)
			{
				newImg.at<cv::Vec3b>(r, c) = pixel;
			}
		}
	}


	printf("Transformation succeeded!");
	return newImg;
}

int main(int argc, char** argv) {
	int width = 300;
	int height = 300;


	Mat img(height, width, CV_8UC3, makeVectorGrey(Scalar(0, 200, 0)));
	cv::Point p1(0, 0);
	cv::Point p2(50, 50);
	circle(img, p1, 200, makeVectorGrey(cv::Scalar(150, 0, 0)), cv::FILLED);
	circle(img, p2, 100, makeVectorGrey(cv::Scalar(255, 255, 255)));

	cv::Mat scale = cv::Mat::eye(3, 3, CV_32F);
	float xscale = 1 / sqrt(2);
	float yscale = 1 / sqrt(2); 
	scale.at<float>(0, 0) = xscale;
	scale.at<float>(1, 1) = yscale;

	cv::Mat trans = cv::Mat::eye(3, 3, CV_32F);
	float xoff = -img.cols / 2.0f;
	float yoff = -img.rows / 2.0f;
	trans.at<float>(0, 2) = xoff;
	trans.at<float>(1, 2) = yoff;

	cv::Mat transBack = cv::Mat::eye(3, 3, CV_32F);
	xoff = img.cols / 2.0f; 
	yoff = img.rows / 2.0f;
	transBack.at<float>(0, 2) = xoff;
	transBack.at<float>(1, 2) = yoff;

	float angle = 45 * CV_PI / 180.0f;
	cv::Mat rot = cv::Mat::eye(3, 3, CV_32F);
	rot.at<float>(0, 0) = cos(angle);
	rot.at<float>(0, 1) = -sin(angle);
	rot.at <float>(1, 0) = sin(angle);
	rot.at<float>(1, 1) = cos(angle);

	Mat transformMatrix = transBack * scale * rot * trans;
	Mat newImg = Mat::zeros(img.size(), img.type());
	cv::warpPerspective(img, newImg, transformMatrix, Size(300, 300));

	for (;;)
	{
		imshow(window_name, img);
		char c = (char)waitKey(0);
		if (c == 27) {
			imwrite("C:\\Projects\\Week2Tasks\\starry_night_grey_remastered.jpg", img);
			printf("Saved into C:\\Projects\\Week2Tasks\\starry_night_grey_remastered.jpg!\n");
			break;
		}
		else if (c == 'i') {
			printf("This is task 3 file :D\n");
		}
	}
	return 0;
}