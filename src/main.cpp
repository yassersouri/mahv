#include <iostream>
#include <string>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <ncurses.h>


using namespace std;

int main(int argc, char **argv) {

	initscr();
	printw("Salam");
	refresh();
	getch();
	endwin();

	return 0;
}

