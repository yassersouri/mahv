#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include <opencv2/opencv.hpp>
#include "ncursesHelper.cpp"
#include "choicesHelper.cpp"
#include "mahv.cpp"

using namespace std;

int main(int argc, char **argv) {
	vector<string> *mainChoices = getMainChoices();
	vector<string> *benchChoices = getBenchNames();

	string welcomString = "Use arrays to select\nSelect What action you like to make";
	string statusString = "You choose number %d with choice name: %s\n";
	string bw = "Use arrays to select\nSelect What benchmark to run";

	const char *ws = welcomString.c_str();
	const char *ss = statusString.c_str();
	const char *bwcs = bw.c_str();

	if (argc > 1) {
		cout << "going to be implemented" << endl;
	} else {
		int choice = runCurses(ws, ss, mainChoices);
		if (choice == 1) {
			int choice = runCurses(bwcs, ss, benchChoices);
			string benchName = benchChoices->at(choice - 1);
			cv::Mat orig, mask, mask255, result, origBU;

			// retrieve the original image and mask from the benchmark set.
			// mask255 is for display purposes.
			getOrigMask(benchName, orig, mask, mask255);

			//back up the original image
			orig.copyTo(origBU);

			// do the actual work
			doMahv(orig, mask, result, 17);

			// display them all
			cv::imshow("orig", origBU);
			cv::imshow("result", result);
			cv::waitKey(0);
		} else {
			cout << "Exit Selected!" << endl;
			return 0;
		}
	}
	return 0;
}

