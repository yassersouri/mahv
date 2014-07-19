#include <stdio.h>
#include <stdlib.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>


using namespace std;

int main(int argc, char **argv) {

	string imgadr;

	if (argc > 1) {
		imgadr = argv[1];
	} else {
		imgadr = "/Users/yasser/sci-repo/opencv/samples/cpp/pic4.png";
	}

	cv::Mat img = cv::imread(imgadr, 1);

	if (img.empty()) {
		cerr << "PROB: Cannot Load File" << endl;
		return 1;
	}

	cv::imshow("test", img);
	cv::waitKey(0);

	return 0;
}

