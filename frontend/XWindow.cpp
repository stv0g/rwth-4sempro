#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "XWindow.h"

Display * XWindow::display = NULL;
int XWindow::windows = 0;
int XWindow::screen = 0;

XException::XException(const char *reason) {
	fprintf(stderr, "%s\n", reason);
	exit(EXIT_FAILURE);
}

XWindow::XWindow(const char *title, int x, int y, int width, int height, unsigned long background) {
	Window rootWindow;

	screen = XDefaultScreen(display);
	rootWindow = XRootWindow(display, screen);

	window = XCreateSimpleWindow(display, rootWindow, x, y, width, height, 0, 0, background);

	XStoreName(display, window, title);
	XSelectInput(display, window, ExposureMask | ButtonPressMask);
	XMapWindow(display, window);
}

XWindow::~XWindow() {
	windows--;

	if (windows == 0) {
		disconnect();
	}
}

Display * XWindow::getDisplay() {
	if (display == NULL) {
		throw new XException("There exists no connection to an XServer!");
	}

	return display;
}

void XWindow::connect(std::string display_name) {
	display = XOpenDisplay(display_name.c_str());

	if (display == NULL) {
		throw XException("Cannot open display!");
	}
}

void XWindow::disconnect() {
	XCloseDisplay(display);
}

Cairo::RefPtr<XWindow> XWindow::create(const char *title, int x, int y, int width, int height, unsigned long background) {
	XWindow *win = new XWindow(title, x, y, width, height, background);
	return Cairo::RefPtr<XWindow>(win);
}
