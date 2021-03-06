#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <termios.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <string.h>
#include <signal.h>
#include "yetitui_element.h"

static struct termios orig_termios;

void disMode_raw();

void enMode_raw() {
	tcgetattr(STDIN_FILENO, &orig_termios);
	atexit(disMode_raw);
	struct termios raw = orig_termios;
	raw.c_lflag &= ~(ECHO);
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void disMode_raw() {
	tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void curs_move(uint16_t x, uint16_t y) {
	printf("\033[%i;%if", y, x);
}

void scr_clear() {
	printf("\033[H\033[J");
}

void setColour(uint8_t colour) {
	printf("\x1b[%im", colour);
}

void setColourBright(uint8_t colour) {
	printf("\x1b[%i;1m", colour);
}

void setColourForeRGB(uint8_t r, uint8_t g, uint8_t b) {
	printf("\x1b[38;2;%i;%i;%im", r, g, b);
}

void setColourBackRGB(uint8_t r, uint8_t g, uint8_t b) {
	printf("\x1b[48;2;%i;%i;%im", r, g, b);
}

void resetColour() {
	printf("\x1b[0m");
}

void printBox(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool border) {
	curs_move(x, y);
	if (border) putchar('+');
	for (int i = 0; i < w - 2; ++i) {
		if (border) putchar('-');
	}
	if (border) putchar('+');
	for (int i = 0; i < h - 2; ++i) {
		curs_move(x, y + i + 1);
		if (border) putchar('|');
		for (int j = 0; j < w - 2; ++j) {
			putchar(' ');
		}
		if (border) putchar('|');
	}
	curs_move(x, y + h - 1);
	if (border) putchar('+');
	for (int i = 0; i < w - 2; ++i) {
		if (border) putchar('-');
	}
	if (border) putchar('+');
}

void printBoxWithElements(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool border, box_element elements[], size_t elementArraySize) {
	bool elementInPlace;
	char element;
	curs_move(x, y);
	if (border) putchar('+');
	for (int i = 0; i < w - 2; ++i) {
		if (border) putchar('-');
	}
	if (border) putchar('+');
	for (int i = 0; i < h - 2; ++i) {
		curs_move(x, y + i + 1);
		if (border) putchar('|');
		for (int j = 0; j < w - 2; ++j) {
			elementInPlace = false;
			for (int k = 0; k<elementArraySize; ++k) {
				if ((j == elements[k].x) && (i == elements[k].y)) {
					elementInPlace = true;
					element        = elements[k].content;
				}
			}
			if (elementInPlace) {
				if (border) putchar(element);
			}
			else {
				if (border) putchar(' ');
			}
		}
		if (border) putchar('|');
	}
	curs_move(x, y + h - 1);
	if (border) putchar('+');
	for (int i = 0; i < w - 2; ++i) {
		if (border) putchar('-');
	}
	if (border) putchar('+');
}

int getmaxy() {
    struct winsize window;
    ioctl(0, TIOCGWINSZ, &window);
    return window.ws_row;
}

int getmaxx() {
    struct winsize window;
    ioctl(0, TIOCGWINSZ, &window);
    return window.ws_col;
}

void showCursor(bool show) {
	if (show) {
		fputs("\e[?25h", stdout);
	}
	else {
		fputs("\e[?25l", stdout);
	}
}

char getch() {
	struct termios info;
	tcgetattr(0, &info);          // get current terminal attirbutes; 0 is the file descriptor for stdin
	info.c_lflag &= ~ICANON;      // disable canonical mode
	info.c_cc[VMIN] = 1;          // wait until at least one keystroke available
	info.c_cc[VTIME] = 0;         // no timeout
	tcsetattr(0, TCSANOW, &info); // set immediately
	char in;
	read(0, &in, 1);
	return in;
}

box_element* eputs(uint16_t x, uint16_t y, const char str[]) {
	box_element* ret     = (box_element*) malloc(strlen(str)*sizeof(box_element));
	box_element* pointer = ret + 1;
	box_element  add;
	int j = x;
	for (int i = 0; i<strlen(str); ++i) {
		add.x            = j;
		add.y            = y;
		add.content      = str[i];
		*pointer         = add;
		++ pointer;
		++ j;
	}
	return ret;
}

box_element createElement(uint16_t x, uint16_t y, char content) {
	box_element ret;
	ret.x       = x;
	ret.y       = y;
	ret.content = content;
	return ret;
}

box_element* ePrintBox(uint16_t x, uint16_t y, uint16_t w, uint16_t h, bool border) {
	box_element* ret = (box_element*) malloc(w*h*sizeof(box_element));
	box_element* pointer = ret + 1;
	for (int i = y; i<h+y; ++i) {
		for (int j = x; j<=w+x; ++i) {
			if (i == y) {
				if (((j == x) || (j == x+w)) && ((i == y) || i == y+h)) {
					if (border) {
						*pointer = createElement(j, i, '+');
						++ pointer;
					}
					else {
						*pointer = createElement(j, i, ' ');
					}
				}
				else if ((j == x) || (j == x+w)) {
					if (border) {
						*pointer = createElement(j, i, '|');
						++ pointer;
					}
					else {
						*pointer = createElement(j, i, ' ');
					}
				}
				else if ((i == y) || (i == y+h)) {
					if (border) {
						*pointer = createElement(j, i, '-');
						++ pointer;
					}
					else {
						*pointer = createElement(j, i, ' ');
					}
				}
				else {
					*pointer = createElement(j, i, ' ');
					++ pointer;
				}
			}
		}
	}
	free(pointer);
	return ret;
}

void endscr(int hi) {
	scr_clear();
	curs_move(1, 1);
	showCursor(true);
	resetColour();
	disMode_raw();
	exit(0);
}

void endscrexit(void) {
	scr_clear();
	curs_move(1, 1);
	showCursor(true);
	resetColour();
	disMode_raw();
	exit(0);
}

void initscr() {
	signal(SIGINT, endscr);
	atexit(endscrexit);
}

void drawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, char fill) {
	int mx = 0, my = 0;
	for (int i = x; i<+x+w; ++i) {
		for (int j = y; j<h+y; ++i) {
			curs_move(i, j);
			putchar(fill);
		}
		putchar(10);
	}
}