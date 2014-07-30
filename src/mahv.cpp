/*
 * mahv.cpp
 *
 *  Created on: Mordad 8, 1393 AP
 *      Author: Yasser Souri
 *
 *  This is an implementation of A. Criminisi, et al, "Region Filling
 *  and Object Removal by Exemplar-Based Image Inpainting", TIP 2004
 */


#include <opencv2/opencv.hpp>
#include <iostream>

using namespace cv;
using namespace std;

cv::Mat calculateFillFront(cv::Mat &mask);
cv::SparseMat calculateSparseFillFront(cv::Mat &fillFront);
void doMahv(cv::Mat orig, cv::Mat mask, cv::Mat &result, int windowSize);

cv::Mat calculateFillFront(cv::Mat &mask) {
	cv::Mat diskElem = cv::Mat::ones(cv::Size(3, 3), CV_8UC1);
	diskElem.at<unsigned char>(0, 0) = 0;
	diskElem.at<unsigned char>(0, 2) = 0;
	diskElem.at<unsigned char>(2, 0) = 0;
	diskElem.at<unsigned char>(2, 2) = 0;

	cv::Mat result, maskEroded;

	cv::erode(mask, maskEroded, diskElem);
	cv::subtract(mask, maskEroded, result);
	return result;
}

cv::SparseMat calculateSparseFillFront(cv::Mat &fillFront) {
	cv::SparseMat result(fillFront);
	return result;
}

void doMahv(cv::Mat orig, cv::Mat mask, cv::Mat &result, int windowSize = 9) {
	cv::Mat origFloat, origCIE, maskInv, fillFront, image_padded, mask_padded, mask_max_1, confidence;
	cv::Mat temp; //FIXME: delete this line
	result = cv::Mat::zeros(orig.rows, orig.cols, CV_8UC3);

	// calculate inverse of the mask
	cv::bitwise_not(mask, maskInv);
	//clear the masked parts of the image
	orig.setTo(cv::Scalar(0), mask);
	// copy non masked part of orig to result
	orig.copyTo(result, maskInv);
	// make a float version of orig image for calculating patch distance.
	orig.convertTo(origFloat, CV_32FC3);

	int offset = windowSize / 2;

	//padding
	cv::copyMakeBorder(origFloat, image_padded, offset, offset, offset, offset, cv::BORDER_CONSTANT, cv::Scalar(0));
	cv::copyMakeBorder(mask, mask_padded, offset, offset, offset, offset, cv::BORDER_CONSTANT, cv::Scalar(0));

	//initial confidence
	mask.convertTo(mask_max_1, CV_8UC1, 1./255, 0);
	cv::subtract(cv::Scalar(1), mask_max_1, confidence);

	while( true ) {
		fillFront = calculateFillFront(mask_max_1);

		if (cv::sum(fillFront) == cv::Scalar(0)) {
			if (cv::sum(mask_max_1) > cv::Scalar(0)) {
				// if the remaining mask region is 1 pixel thick.
				fillFront = mask_max_1;

				// just checking it for my self
				cout << "URGENT CHECK IF THIS IS CORRECT" << endl;
				cv::imshow("fillFront", fillFront);
				cv::waitKey(0);
			} else {
				break;
			}
		}

		// convert image_padded to grayscale and calculate gradients
		// or calculate gradients in each channel and add them together
		// which one is faster?
	}

	//FIXME: copy the non-padded parts of image_padded to result
}
