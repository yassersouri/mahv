#include <opencv2/opencv.hpp>

using namespace cv;

void extractMaskFromBench(Mat &maskImg, Mat &mask, unsigned char mask_val=1, unsigned char red_color_min=224, unsigned char other_color_max=31) {
	mask = cv::Mat::zeros(maskImg.size(), CV_8UC1);

	for (int i = 0; i < maskImg.rows; ++i) {
		for (int j = 0; j < maskImg.cols; ++j) {
			Vec3b val = maskImg.at<Vec3b>(i, j);
			if (val[2] >= red_color_min && val[0] <= other_color_max && val[1] <= other_color_max) {
				mask.at<unsigned char>(i, j) = mask_val;
			}
		}
	}
}
