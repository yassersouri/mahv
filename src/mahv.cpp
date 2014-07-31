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
const cv::SparseMat::Node *calculateMaxPriorityPoint(cv::Mat &fillFront, cv::Mat &confidence, cv::Mat &dx, cv::Mat &dy, cv::Mat &nx, cv::Mat &ny, int windowSize, float alpha);
float calculateDataTerm(cv::Mat &dx, cv::Mat &dy, cv::Mat &nx, cv::Mat &ny, int i, int j, int offset, float alpha);
float calculateConfidenceTerm(cv::Mat &confidence, int i, int j, int windowSize, int offset);
void visConfidence(cv::Mat &confidence, cv::Mat &orig, cv::Mat &mask, int windowSize);
void visDataTerm(cv::Mat &dataTerm, cv::Mat &orig, cv::Mat &mask, int windowSize, float alpha);
void findMinDiffPatch(cv::Mat &orig, cv::Mat &mask, cv::Mat &imageTemplate, cv::Mat &maskTemplate, int i, int j, int windowSize, int offset, int &i_m, int &j_m);

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
	int i_data = 0, j_data = 0;
	float max_mod = 0, data_term;

	for (int k = -offset; k <= offset; ++k) {
		for (int l = -offset; l <= offset; ++l) {
			int n_i = i + k;
			int n_j = j + l;
			float mod = pow(dx.at<float>(n_i, n_j), 2) + pow(dy.at<float>(n_i, n_j), 2);
			if (mod > max_mod) {
				max_mod = mod;
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

const cv::SparseMat::Node *calculateMaxPriorityPoint(cv::Mat &fillFront, cv::Mat &confidence, cv::Mat &dx, cv::Mat &dy, cv::Mat &nx, cv::Mat &ny, int windowSize, float alpha=255) {
	float max_priority = -1;
	const cv::SparseMat::Node *max_point;

	int offset = windowSize / 2;
	cv::SparseMat sparseFillFront = calculateSparseFillFront(fillFront);
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
		float conf_term = calculateConfidenceTerm(confidence, i, j, windowSize, offset);
		confidence.at<float>(i, j) = conf_term;

		float priority = data_term * conf_term;

		if (priority > max_priority) {
			max_priority = priority;
			max_point = node;
		}
	}

	return max_point;
}

void findMinDiffPatch(cv::Mat &orig, cv::Mat &mask, cv::Mat &imageTemplate, cv::Mat &maskTemplate, int i, int j, int windowSize, int offset, int &i_m, int &j_m) {
	int i_min = -1;
	int j_min = -1;
	float min_ssd = MAXFLOAT;
	cv::Mat candidSourcePatch, ssdAbsDiff, ssdSquaredDiff, maskTemplateInv, maskTemplateInvMC, maskTemplateInvMCFloat;
	cv::bitwise_not(maskTemplate, maskTemplateInv);
	const cv::Mat masktempprep[3] = {maskTemplateInv, maskTemplateInv, maskTemplateInv};
	cv::merge(masktempprep, 3, maskTemplateInvMC);
	maskTemplateInvMC.convertTo(maskTemplateInvMCFloat, CV_32FC3);
	for(int k = offset; k < orig.rows - offset; ++k) {
		for (int l = offset; l < orig.cols - offset; ++l) {
			//check that this candidate source patch does not have overlap with target patch
			if (abs(k - i) < windowSize || abs(l - j) < windowSize) {
				// target and source patch overlap
				continue;
			}
			if (cv::sum(mask.rowRange(k - offset, k + offset + 1)
							.colRange(l - offset, l + offset + 1))[0] > 0 ) {
				// target region overlaps with masked region
				continue;
			}
			candidSourcePatch = orig.rowRange(k - offset, k + offset + 1)
										.colRange(l - offset, l + offset + 1);
			cv::absdiff(imageTemplate, candidSourcePatch, ssdAbsDiff);
			cv::multiply(ssdAbsDiff, maskTemplateInvMCFloat, ssdAbsDiff);
			cv::multiply(ssdAbsDiff, ssdAbsDiff, ssdSquaredDiff);
			cv::Scalar ssdSum = cv::sum(ssdSquaredDiff);
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
	cv::Mat origFloat, maskInv, fillFront, image_padded, mask_padded, mask_max_1, confidence;
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
	mask_padded.convertTo(mask_max_1, CV_8UC1, 1./255, 0);
	cv::subtract(cv::Scalar(1), mask_max_1, confidence);
	confidence.convertTo(confidence, CV_32FC1);
	int iter = 0;
	while( true ) {
		fillFront = calculateFillFront(mask_max_1);

		if (cv::sum(fillFront)[0] == 0) {
			if (cv::sum(mask_max_1)[0] > 0) {
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
		cv::Mat dx, dy, nx, ny, image_padded_gray, imageTemplate, maskTemplate;
		cv::cvtColor(image_padded, image_padded_gray, COLOR_BGR2GRAY);
		cv::Sobel(image_padded_gray, dx, CV_32F, 1, 0, 3, 1, 0, cv::BORDER_CONSTANT);
		cv::Sobel(image_padded_gray, dy, CV_32F, 0, 1, 3, -1, 0, cv::BORDER_CONSTANT);
		cv::Sobel(mask_padded, nx, CV_32F, 1, 0, 3, 1, 0, cv::BORDER_CONSTANT);
		cv::Sobel(mask_padded, ny, CV_32F, 0, 1, 3, 1, 0, cv::BORDER_CONSTANT);

		dx.setTo(cv::Scalar(0), mask_padded);
		dy.setTo(cv::Scalar(0), mask_padded);

		//calculate the point on the fill front with the maximum priority
		const cv::SparseMat::Node *p = calculateMaxPriorityPoint(fillFront, confidence, dx, dy, nx, ny, windowSize, 255);

		int i = p->idx[0];
		int j = p->idx[1];

		//get the patches around the found template
		cv::Range targetRowRange_padded(i - offset, i + offset + 1);
		cv::Range targetColRange_padded(j - offset, j + offset + 1);
//		cv::circle(result, cv::Point(j - offset, i - offset), offset, cv::Scalar(0, 0, 255));
		imageTemplate = image_padded.rowRange(targetRowRange_padded).colRange(targetColRange_padded);
		maskTemplate = mask_padded.rowRange(targetRowRange_padded).colRange(targetColRange_padded);

		//find the best part in the source region of the image
		int i_m, j_m;
		findMinDiffPatch(origFloat, mask, imageTemplate, maskTemplate, i - offset, j - offset, windowSize, offset, i_m, j_m);

		if (i_m == -1 || j_m == -1) {
			cout << "NO PATCH FOUND" << endl;
			exit(1);
		} else {
			cv::Range sourceRowRange(i_m - offset, i_m + offset + 1);
			cv::Range sourceColRange(j_m - offset, j_m + offset + 1);
			cv::Range sourceRowRange_padded(sourceRowRange.start + offset, sourceRowRange.end + offset);
			cv::Range sourceColRange_padded(sourceColRange.start + offset, sourceColRange.end + offset);
			cv::Range targetRowRange(targetRowRange_padded.start - offset, targetRowRange_padded.end - offset);
			cv::Range targetColRange(targetColRange_padded.start - offset, targetColRange_padded.end - offset);

			result.rowRange(sourceRowRange).colRange(sourceColRange).copyTo(result.rowRange(targetRowRange).colRange(targetColRange));
			image_padded.rowRange(sourceRowRange_padded).colRange(sourceColRange_padded).copyTo(image_padded.rowRange(targetRowRange_padded).colRange(targetColRange_padded));
			mask.rowRange(targetRowRange).colRange(targetColRange).setTo(cv::Scalar(0));
			mask_max_1.rowRange(targetRowRange).colRange(targetColRange).setTo(cv::Scalar(0));
			mask_padded.rowRange(targetRowRange_padded).colRange(targetColRange_padded).setTo(cv::Scalar(0));
			confidence.rowRange(targetRowRange_padded).colRange(targetColRange_padded).setTo(confidence.at<float>(i, j));
		}
		++iter;
		cout << iter << endl;
		if (iter % 100 == 0) {
			cv::imshow("result", result);
			cv::imshow("mask", mask);
			cv::waitKey(0);
		}
	}

	//FIXME: copy the non-padded parts of image_padded to result
}
