#include <string>
#include <vector>
#include <ncurses.h>

using namespace std;

void print_menu(WINDOW *menu_win, int highlight, vector<string> *choices);

int runCurses(const char *welcomeString, const char *statusString, vector<string> *choices) {
	WINDOW *menu_win;
	int highlight = 1;
	int choice = 0;
	int c;
	bool done = false;

	initscr();
	clear();
	noecho();
	cbreak();
	int xleave = 6;
	int yleave = 10;
	int WIDTH;
	int HEIGHT;
	getmaxyx(stdscr, HEIGHT, WIDTH);
	HEIGHT = HEIGHT - xleave;
	WIDTH = WIDTH - yleave;
	int startx = (xleave) / 2;
	int starty = (yleave) / 2;

	menu_win = newwin(HEIGHT, WIDTH, starty, startx);
	keypad(menu_win, TRUE);
	mvprintw(0, 0, welcomeString);
	refresh();
	print_menu(menu_win, highlight, choices);
	while(1) {
		c = wgetch(menu_win);
		switch (c) {
			case KEY_UP:
				if (highlight == 1)
					highlight = choices->size();
				else
					--highlight;
				mvprintw(starty + HEIGHT, 0, statusString, highlight, choices->at(highlight - 1).c_str());
				refresh();
				break;
			case KEY_DOWN:
				if (highlight == choices->size())
					highlight = 1;
				else
					++highlight;
				mvprintw(starty + HEIGHT, 0, statusString, highlight, choices->at(highlight - 1).c_str());
				refresh();
				break;
			case 10:
				choice = highlight;
				done = true;
				break;
		}
		print_menu(menu_win, highlight, choices);
		if (done)
			break;
	}
	clrtoeol();
	refresh();
	endwin();
	return choice;
}

void print_menu(WINDOW *menu_win, int highlight, vector<string> *choices) {
	int x, y, i;

	x = 2;
	y = 2;

	box(menu_win, 0, 0);

	for (i = 0; i < choices->size(); ++i) {
		if (highlight == i + 1) {
			wattron(menu_win, A_REVERSE);
			mvwprintw(menu_win, y, x, "%s", choices->at(i).c_str());
			wattroff(menu_win, A_REVERSE);
		} else {
			mvwprintw(menu_win, y, x, "%s", choices->at(i).c_str());
		}
		++y;
	}
	wrefresh(menu_win);
}
