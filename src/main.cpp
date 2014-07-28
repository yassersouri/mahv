#include <iostream>
#include <string>
#include <vector>
#include <ncurses.h>

using namespace std;

void print_menu(WINDOW *menu_win, int highlight);

#define WIDTH 40
#define HEIGHT 10

//char *choices[] = {
//	"Choice 1",
//	"Choice 2",
//	"Choice 3",
//	"Exit",
//};

vector<string> *choices = new vector<string>();
int n_choices;

void runCurses() {
	WINDOW *menu_win;
	int highlight = 1;
	int choice = 0;
	int c;

	initscr();
	clear();
	noecho();
	cbreak();
	int startx = (80 - WIDTH) / 2;
	int starty = (24 - HEIGHT) / 2;

	menu_win = newwin(HEIGHT, WIDTH, starty, startx);
	keypad(menu_win, TRUE);
	mvprintw(0, 0, "Use arrows");
	refresh();
	print_menu(menu_win, highlight);
	while(1) {
		c = wgetch(menu_win);
		switch (c) {
			case KEY_UP:
				if (highlight == 1)
					highlight = n_choices;
				else
					--highlight;
				break;
			case KEY_DOWN:
				if (highlight == n_choices)
					highlight = 1;
				else
					++highlight;
				break;
			case 10:
				choice = highlight;
				break;
			default:
				mvprintw(24, 0, "Character pressed is = %3d Hopefully it can be printed as '%c'", c, c);
				refresh();
				break;
		}
		print_menu(menu_win, highlight);
		if (choice != 0)
			break;
	}
	mvprintw(23, 0, "You choose choice %d with choice string %s\n", choice, choices->at(choice - 1).c_str());
	clrtoeol();
	getch();
	refresh();
	endwin();

	cout << "ENDED" << endl;
}

void print_menu(WINDOW *menu_win, int highlight) {
	int x, y, i;

	x = 2;
	y = 2;

	box(menu_win, 0, 0);

	for (i = 0; i < n_choices; ++i) {
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

int main(int argc, char **argv) {
	choices->push_back("Choice 1");
	choices->push_back("Choice 2");
	choices->push_back("Choice 3");
	choices->push_back("Exit");

	n_choices = choices->size();
	if (argc > 1) {
		cout << "going to be implemented" << endl;
	} else {
		runCurses();
	}

	return 0;
}

