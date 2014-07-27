#include <iostream>
#include <string>
#include <ncurses.h>

using namespace std;

void runCurses() {
	int ch;
	initscr();
	raw();
	keypad(stdscr, true);
	noecho();

	printw("Type \n");

	ch = getch();

	if (ch == KEY_F(1)) {
		printw("F1 Pressed");
	} else {
		printw("The key pressed is ");
		attron(A_BOLD);
		printw("%c", ch);
		attroff(A_BOLD);
	}

	refresh();
	getch();
	endwin();
}

int main(int argc, char **argv) {

	if (argc > 1) {
		cout << "going to be implemented" << endl;
	} else {
		runCurses();
	}

	return 0;
}

