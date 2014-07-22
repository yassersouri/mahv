#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <map>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace std;
using namespace cv;

const char MODE_NO_SELECTION = 'n';
const char MODE_SELECTION = 's';
const char MODE_DONE = 'd';
const char ACTION_UNDO = 'u';

map<char, string> *modeDescriptions = new map<char, string>;
char mode = MODE_NO_SELECTION;
vector<Point> *points = new vector<Point>();
int keyPressed = -1;
char charTyped = ' ';

Mat imgOriginal, imgDup, imgDraw;
string guiWindowName = "Select Target Region";

void draw(Mat &imgo, Mat &img, vector<Point> *points) {
	bool drawPoints = false;
	imgo.copyTo(img);

	Mat mask(imgo.rows, imgo.cols, CV_8UC3, Scalar(0, 0, 0));
	Mat White(imgo.rows, imgo.cols, CV_8UC3, Scalar(255, 255, 255));

	const int size = points->size();
	Point *newPoints = new Point[size];

	for (int i = 0; i < points->size(); ++i) {
		Point thePoint = points->at(i);
		newPoints[i] = thePoint;
		if (drawPoints) {
			circle(img, thePoint, 3, Scalar(30, 25, 200), 2);
		}
	}
	fillPoly(mask, (const Point**) &newPoints, &size, 1, Scalar(255, 255, 255));

	Mat notMask, notMaskPart, nonMaskFinal;
	bitwise_not(mask, notMask);
	bitwise_and(imgo, notMask, notMaskPart);
	add(mask, notMaskPart, nonMaskFinal);

	float alpha = 0.2;
	float beta = 1 - alpha;

	addWeighted(imgo, alpha, nonMaskFinal, beta, 0.0, img);
}

static void onMouse(int event, int x, int y, int, void*) {
	switch (mode) {
		case MODE_NO_SELECTION:
			if (event == EVENT_LBUTTONDOWN) {
				mode = MODE_SELECTION;
				points->push_back(Point(x, y));
			}
			break;
		case MODE_SELECTION:
			if (event == EVENT_LBUTTONDOWN) {
				points->push_back(Point(x, y));
			} else if(event == EVENT_RBUTTONDOWN) {
				mode = MODE_DONE;
			}
			break;
		default:
			break;
	}
	draw(imgDup, imgDraw, points);
	imshow(guiWindowName, imgDraw);

	// FIXME: Debug output, remove later
	cout << mode << '|' << points->size() << endl;
}

int main(int argc, char **argv) {

	string imgadr;

	if (argc > 1) {
		imgadr = argv[1];
	} else {
		imgadr = "/Users/yasser/sci-repo/opencv/samples/cpp/pic4.png";
	}

	imgOriginal = imread(imgadr, 1);

	if (imgOriginal.empty()) {
		cerr << "PROB: Cannot Load File" << endl;
		return 1;
	}

	imgOriginal.copyTo(imgDup);
	imgOriginal.copyTo(imgDraw);

	namedWindow(guiWindowName, WINDOW_AUTOSIZE);
	setMouseCallback(guiWindowName, onMouse);
	moveWindow(guiWindowName, 200, 300);

	while(1) {
		imshow(guiWindowName, imgDraw);
		keyPressed = waitKey(0);

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
