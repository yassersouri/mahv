#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <map>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace std;

const char MODE_NO_SELECTION = 'n';
const char MODE_SELECTION = 's';
const char MODE_DONE = 'd';

const char ACTION_UNDO = 'u';

map<char, string> *modeDescriptions = new map<char, string>;

char mode = MODE_NO_SELECTION;
vector<cv::Point_<int> > *points = new vector<cv::Point_<int> >();
int keyPressed = -1;
char charTyped = ' ';

cv::Mat imgOriginal, imgDup, imgDraw;
string guiWindowName = "Select Target Region";

void draw(cv::Mat &imgo, cv::Mat &img, vector<cv::Point_<int> > *points) {
	imgo.copyTo(img);
//	cv::fillPoly(img, (const cv::Point**) points, new array[points->size(), 1, cv::Scalar(0, 0, 255), 0, 0, 0);
	for (int i = 0; i < points->size(); ++i) {
		cv::circle(img, points->at(i), 3, cv::Scalar(0, 0, 255), 3);
	}
}

static void onMouse(int event, int x, int y, int, void*) {
	switch (mode) {
		case MODE_NO_SELECTION:
			if (event == cv::EVENT_LBUTTONDOWN) {
				mode = MODE_SELECTION;
				points->push_back(cv::Point_<int>(x, y));
			}
			break;
		case MODE_SELECTION:
			if (event == cv::EVENT_LBUTTONDOWN) {
				points->push_back(cv::Point_<int>(x, y));
			} else if(event == cv::EVENT_RBUTTONDOWN) {
				mode = MODE_DONE;
			}
			break;
		default:
			break;
	}
	draw(imgDup, imgDraw, points);
	cv::imshow(guiWindowName, imgDraw);
	cout << mode << '|' << points->size() << endl;
}

int main(int argc, char **argv) {

	string imgadr;

	if (argc > 1) {
		imgadr = argv[1];
	} else {
		imgadr = "/Users/yasser/sci-repo/opencv/samples/cpp/pic4.png";
	}

	imgOriginal = cv::imread(imgadr, 1);

	if (imgOriginal.empty()) {
		cerr << "PROB: Cannot Load File" << endl;
		return 1;
	}

	imgOriginal.copyTo(imgDup);
	imgOriginal.copyTo(imgDraw);

	cv::namedWindow(guiWindowName, cv::WINDOW_AUTOSIZE);
	cv::setMouseCallback(guiWindowName, onMouse);
	cv::moveWindow(guiWindowName, 200, 300);

	while(1) {
		cv::imshow(guiWindowName, imgDraw);
		keyPressed = cv::waitKey(0);

		bool toExit = false;
		if ((keyPressed & 255) == 27) {
			switch (mode) {
				case MODE_SELECTION:
				case MODE_DONE:
					mode = MODE_NO_SELECTION;
					points->clear();
					break;
				case MODE_NO_SELECTION:
					toExit = true;
					break;
				default:
					break;
			}
			if (toExit) {
				break;
			}
		} else {
			charTyped = (char) keyPressed;
			switch (charTyped) {
				case ACTION_UNDO:
					if (mode == MODE_SELECTION) {
						points->pop_back();
						if (points->size() == 0) {
							mode = MODE_NO_SELECTION;
						}
					}
					break;
				default:
					break;
			}
		}
		draw(imgDup, imgDraw, points);
	}

	return 0;
}
