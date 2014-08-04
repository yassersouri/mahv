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
#include <vector>
#include <iostream>
#include <cmath>

using namespace cv;
using namespace std;

cv::Mat calculateFillFront(cv::Mat &mask);
cv::SparseMat calculateSparseFillFront(cv::Mat &fillFront);
void doMahv(cv::Mat &orig, cv::Mat &mask, cv::Mat &result, int windowSize);
const cv::SparseMat::Node *calculateMaxPriorityPoint(cv::Mat &fillFront, cv::Mat &confidence_padded, cv::Mat &dx, cv::Mat &dy, cv::Mat &nx, cv::Mat &ny, int windowSize, float alpha);
float calculateDataTerm(cv::Mat &dx, cv::Mat &dy, cv::Mat &nx, cv::Mat &ny, int i, int j, int offset, float alpha);
float calculateConfidenceTerm(cv::Mat &confidence, int i, int j, int windowSize, int offset);
void visConfidence(cv::Mat &confidence, cv::Mat &orig, cv::Mat &mask, int windowSize);
void visDataTerm(cv::Mat &dataTerm, cv::Mat &orig, cv::Mat &mask, int windowSize, float alpha);
void findMinDiffPatch(cv::Mat &origCIE, cv::Mat &mask, cv::Mat &imageTemplate, cv::Mat &maskTemplate, int i, int j, int windowSize, int offset, int &i_m, int &j_m);

void visDataTerm(cv::Mat &dataTerm, cv::Mat &orig, cv::Mat &mask, int windowSize, float alpha) {
	cv::Mat origFloat, origCIE, maskInv, fillFront, image_padded, mask_padded, mask_max_1, confidence;
	orig.convertTo(origFloat, CV_32FC3);
	int offset = windowSize / 2;
	cv::copyMakeBorder(origFloat, image_padded, offset, offset, offset, offset, cv::BORDER_CONSTANT, cv::Scalar(0));
	cv::copyMakeBorder(mask, mask_padded, offset, offset, offset, offset, cv::BORDER_CONSTANT, cv::Scalar(0));

	cv::Mat dx, dy, nx, ny, image_padded_gray;
	cv::cvtColor(image_padded, image_padded_gray, COLOR_BGR2GRAY);
	cv::Sobel(image_padded_gray, dx, CV_32F, 1, 0, 3, 1, 0, cv::BORDER_CONSTANT);
	cv::Sobel(image_padded_gray, dy, CV_32F, 0, 1, 3, -1, 0, cv::BORDER_CONSTANT);
	cv::Sobel(mask_padded, nx, CV_32F, 1, 0, 3, 1, 0, cv::BORDER_CONSTANT);
	cv::Sobel(mask_padded, ny, CV_32F, 0, 1, 3, 1, 0, cv::BORDER_CONSTANT);

	dataTerm = cv::Mat::zeros(image_padded.rows, image_padded.cols, CV_32FC1);

	for (int x = 0; x != mask_padded.rows; ++x) {
		for (int y = 0; y != mask_padded.cols; ++y) {
			if (mask_padded.at<unsigned char>(x, y) != 0) {
				float d_term = calculateDataTerm(dx, dy, nx, ny, x, y, offset, alpha);
				dataTerm.at<float>(x, y) = d_term;
			}
		}
	}
}

void visConfidence(cv::Mat &confidence, cv::Mat &orig, cv::Mat &mask, int windowSize) {
	cv::Mat origFloat, image_padded, mask_padded, mask_max_1;
	orig.convertTo(origFloat, CV_32FC3);
	int offset = windowSize / 2;
	cv::copyMakeBorder(origFloat, image_padded, offset, offset, offset, offset, cv::BORDER_CONSTANT, cv::Scalar(0));
	cv::copyMakeBorder(mask, mask_padded, offset, offset, offset, offset, cv::BORDER_CONSTANT, cv::Scalar(0));
	mask_padded.convertTo(mask_max_1, CV_8UC1, 1./255, 0);
	cv::subtract(cv::Scalar(1), mask_max_1, confidence);
	confidence.convertTo(confidence, CV_32FC1);

	for (int x = 0; x != mask_padded.rows; ++x) {
		for (int y = 0; y != mask_padded.cols; ++y) {
			if (mask_padded.at<unsigned char>(x, y) != 0) {
				float conf_term = calculateConfidenceTerm(confidence, x, y, windowSize, offset);
				confidence.at<float>(x, y) = conf_term;
			}
		}
	}
}



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

float calculateDataTerm(cv::Mat &dx, cv::Mat &dy, cv::Mat &nx, cv::Mat &ny, int i, int j, int offset, float alpha) {
	int i_data = 0,
		j_data = 0;
	float max_edge_power = 0,
		  data_term;

	for (int k = -offset; k <= offset; ++k) {
		for (int l = -offset; l <= offset; ++l) {
			int n_i = i + k;
			int n_j = j + l;

			float edge_power = pow(dx.at<float>(n_i, n_j), 2) + pow(dy.at<float>(n_i, n_j), 2);
			if (edge_power > max_edge_power) {
				max_edge_power = edge_power;
				i_data = n_i;
				j_data = n_j;
			}
		}
	}

	if (i_data == 0 && j_data == 0) {
		// if no variation in intensity, then data term is zero, then priority is zero
		data_term = 0;
	} else {
		data_term = abs(dx.at<float>(i_data, j_data) * nx.at<float>(i, j) +
					  +dy.at<float>(i_data, j_data) * ny.at<float>(i, j));
		data_term /= alpha;
	}

	return data_term;
}

float calculateConfidenceTerm(cv::Mat &confidence, int i, int j, int windowSize, int offset) {
	float conf_term = 0;

	for (int k = -offset; k <= offset; ++k) {
		for (int l = -offset; l <= offset; ++l) {
			conf_term += confidence.at<float>(i + k, j + l);
		}
	}
	conf_term /= (float)(windowSize * windowSize);
	return conf_term;
}

const cv::SparseMat::Node *calculateMaxPriorityPoint(cv::Mat &fillFront_padded, cv::Mat &confidence_padded, cv::Mat &dx, cv::Mat &dy, cv::Mat &nx, cv::Mat &ny, int windowSize, float alpha=255) {
	float max_priority = -1;
	const cv::SparseMat::Node *max_point;

	int offset = windowSize / 2;
	cv::SparseMat sparseFillFront = calculateSparseFillFront(fillFront_padded);
	SparseMatConstIterator
		it = sparseFillFront.begin(),
		it_end = sparseFillFront.end();

	//lets set the results to the first point first
	//later if we find better points it will change
	max_point = it.node();

	for(; it != it_end; ++it) {
		const SparseMat::Node *node = it.node();
		int i = node->idx[0];
		int j = node->idx[1];

		//calculate data terms
		float data_term = calculateDataTerm(dx, dy, nx, ny, i, j, offset, alpha);

		//calculate confidence terms
		float conf_term = calculateConfidenceTerm(confidence_padded, i, j, windowSize, offset);
		confidence_padded.at<float>(i, j) = conf_term;

		float priority = data_term * conf_term;

		if (priority > max_priority) {
			max_priority = priority;
			max_point = node;
		}
	}

	return max_point;
}

void findMinDiffPatch(cv::Mat &image_padded, cv::Mat &mask_padded, cv::Mat &imageTemplate, cv::Mat &maskTemplate, int i, int j, int windowSize, int offset, int &i_m, int &j_m) {
	int i_min = -1;
	int j_min = -1;
	float min_ssd = MAXFLOAT;
	cv::Mat imageTemplateCIE, candidSourcePatch, ssdAbsDiff, ssdSquaredDiff;
	cv::Mat maskTemplateInv, maskTemplateInvFloat, maskTemplateInvFloat3C;

	// we want the inverse of mask's template
	// because the mask template's value is 1 at masked points we cannot use bitwise_not for finding its inverse.
	maskTemplate.convertTo(maskTemplateInv, CV_32F, -1, 1);
	// make the inverse of mask template 3 channels! each channel the same as the other two!
	cv::cvtColor(maskTemplateInvFloat, maskTemplateInvFloat3C, COLOR_GRAY2BGR);

	// prepare the image template to calculate the SSD
	cv::cvtColor(imageTemplate, imageTemplateCIE, COLOR_BGR2XYZ);

	for(int k = windowSize; k < image_padded.rows - windowSize; ++k) {
		for (int l = windowSize; l < image_padded.cols - windowSize; ++l) {
			//check that this candidate source patch does not have overlap with target patch
			if (abs(k - i) < windowSize || abs(l - j) < windowSize) {
				// target and source patch overlap
				continue;
			}

			cv::Range candidSourceRowRange = cv::Range(k - offset, k + offset + 1);
			cv::Range candidSourceColRange = cv::Range(l - offset, l + offset + 1);


			if (cv::sum(mask_padded.rowRange(candidSourceRowRange)
								   .colRange(candidSourceColRange))[0] > 0 ) {
				// target region overlaps with masked region
				continue;
			}
			candidSourcePatch = image_padded.rowRange(candidSourceColRange)
											.colRange(candidSourceColRange);
			// calculate the absolute diff between source patch and target patch
			cv::absdiff(imageTemplateCIE, candidSourcePatch, ssdAbsDiff);
			// multiply the result by inverse of mask to only consider out of mask regions in calculating the ssd.
			cv::multiply(ssdAbsDiff, maskTemplateInvFloat3C, ssdAbsDiff);
			// now calculate squared of absolute diff
			cv::multiply(ssdAbsDiff, ssdAbsDiff, ssdSquaredDiff);
			// calculate the sum, in each of 3 channels
			cv::Scalar ssdSum = cv::sum(ssdSquaredDiff);
			// sum the ssd for all channels
			float ssd = ssdSum[0] + ssdSum[1] + ssdSum[2];

			if (ssd < min_ssd) {
				min_ssd = ssd;
				i_min = k;
				j_min = l;
			}
		}
	}
	i_m = i_min;
	j_m = j_min;
}

void doMahv(cv::Mat &orig, cv::Mat &mask, cv::Mat &result, int windowSize = 9) {
	// windowSize must be odd
	assert(windowSize % 2 == 1);

	cv::Mat origFloat, origCIE, maskInv, fillFront_padded, image_padded, mask_padded, confidence, confidence_padded;
	// FIXME: remove the following line
	cv::Mat mask_temp, image_temp, fillfront_temp;

	result = cv::Mat::zeros(orig.rows, orig.cols, CV_8UC3);

	// calculate inverse of the mask
	mask.convertTo(maskInv, CV_8UC1, -1, 1);
	//clear the masked parts of the image
	orig.setTo(cv::Scalar(0), mask);
	// copy non masked part of orig to result
	orig.copyTo(result, maskInv);
	// make a float version of orig image for calculating patch distance.
	orig.convertTo(origFloat, CV_32FC3);
	// make a CIE color version of input
	cv::cvtColor(origFloat, origCIE, COLOR_BGR2XYZ);

	int offset = windowSize / 2;

	//padding
	cv::copyMakeBorder(origFloat, image_padded, offset, offset, offset, offset, cv::BORDER_CONSTANT, cv::Scalar(0));
	cv::copyMakeBorder(mask, mask_padded, offset, offset, offset, offset, cv::BORDER_CONSTANT, cv::Scalar(0));

	//initial confidence, confidence is (1 - mask) which is maskInv.
	maskInv.convertTo(confidence, CV_32FC1);
	// padding the confidence
	cv::copyMakeBorder(confidence, confidence_padded, offset, offset, offset, offset, cv::BORDER_CONSTANT, cv::Scalar(1));

	int iter = 0;
	while( true ) {
		fillFront_padded = calculateFillFront(mask_padded);

		// FIXME: delete these
		{
			fillFront_padded.convertTo(fillfront_temp, CV_8UC1, 255, 0);
			cv::imshow("fillfront", fillfront_temp);
		}

		if (cv::sum(fillFront_padded)[0] == 0) {
			if (cv::sum(mask)[0] > 0) {
				// if the remaining mask region is 1 pixel thick.
				fillFront_padded = mask_padded;

				// just checking it for my self
				cout << "URGENT CHECK IF THIS IS CORRECT" << endl;
				cv::imshow("fillFront", fillFront_padded);
				cv::waitKey(0);
			} else {
				break;
			}
		}

		// convert image_padded to grayscale and calculate gradients
		cv::Mat dx, dy, nx, ny, image_padded_gray, imageTemplate, maskTemplate;
		cv::cvtColor(image_padded, image_padded_gray, COLOR_BGR2GRAY);
		cv::Sobel(image_padded_gray, dx, CV_32F, 1, 0, 3, 1, 0, cv::BORDER_CONSTANT);
		cv::Sobel(image_padded_gray, dy, CV_32F, 0, 1, 3, -1, 0, cv::BORDER_CONSTANT);
		cv::Sobel(mask_padded, nx, CV_32F, 1, 0, 3, 1, 0, cv::BORDER_CONSTANT);
		cv::Sobel(mask_padded, ny, CV_32F, 0, 1, 3, 1, 0, cv::BORDER_CONSTANT);

		// set the dx and dy to zero in the masked area.
		dx.setTo(cv::Scalar(0), mask_padded);
		dy.setTo(cv::Scalar(0), mask_padded);

		// calculate the point on the fill front with the maximum priority
		const cv::SparseMat::Node *p = calculateMaxPriorityPoint(fillFront_padded, confidence_padded, dx, dy, nx, ny, windowSize, 1 * 255);

		// indices of the pixel on the fill front with maximum priority
		int i = p->idx[0];
		int j = p->idx[1];

		// FIXME: delete these.
		{
			image_padded.convertTo(image_temp, CV_8U);
			cv::circle(image_temp, cv::Point(j, i), offset, cv::Scalar(0, 0, 255));
			cv::imshow("result", image_temp);
			cv::waitKey(0);
		}

		// get the patch around the target
		cv::Range targetRowRange_padded(i - offset, i + offset + 1);
		cv::Range targetColRange_padded(j - offset, j + offset + 1);
		imageTemplate = image_padded.rowRange(targetRowRange_padded).colRange(targetColRange_padded);
		maskTemplate = mask_padded.rowRange(targetRowRange_padded).colRange(targetColRange_padded);

		//find the best part in the source region of the image
		int i_m, j_m;
		findMinDiffPatch(image_padded, mask_padded, imageTemplate, maskTemplate, i, j, windowSize, offset, i_m, j_m);

		// get the patch around the source
		cv::Range sourceRowRange_padded(i_m - offset, i_m + offset + 1);
		cv::Range sourceColRange_padded(j_m - offset, j_m + offset + 1);

		if (i_m == -1 || j_m == -1) {
			cout << "NO PATCH FOUND" << endl;
			exit(1);
		} else {
			cv::circle(image_temp, cv::Point(j_m, i_m), offset, cv::Scalar(0, 0, 255));
			cv::imshow("result", image_temp);
			cv::waitKey(0);

			image_padded.rowRange(sourceRowRange_padded).colRange(sourceColRange_padded).copyTo(image_padded.rowRange(targetRowRange_padded).colRange(targetColRange_padded));
			mask_padded.rowRange(targetRowRange_padded).colRange(targetColRange_padded).setTo(cv::Scalar(0));
			confidence_padded.rowRange(targetRowRange_padded).colRange(targetColRange_padded).setTo(confidence_padded.at<float>(i, j));
		}
		++iter;
		cout << iter << endl;
		if (iter % 1 == 0) {
			cv::imshow("result", image_padded);
			mask_padded.convertTo(mask_temp, CV_8UC1, 255, 0);
			cv::imshow("mask", mask_temp);
			cv::imshow("confidence", confidence_padded);
			cv::waitKey(0);
		}
	}

	//FIXME: copy the non-padded parts of image_padded to result
}
