#ifndef _XWINDOW_H_
#define _XWINDOW_H_

#include <cairomm/refptr.h>
#include <X11/Xlib.h>

#include <string>

class XException {
  public:
	XException(const char * reason);
};

class XWindow {

  public:
	XWindow(
		const char *title,
		int x = 1, int y = 1,
		int width = 800, int height = 600,
		unsigned long background = 0
	);

	virtual ~XWindow();

	Window getWindow() { return window; };
	Visual * getVisual() { return XDefaultVisual(display, screen); };

	static Display * getDisplay();

	static void connect(std::string display);
	static void disconnect();

	static Cairo::RefPtr<XWindow> create(
		const char *title,
		int x = 1, int y = 1,
		int width = 800, int height = 600,
		unsigned long background = 0
	);

protected:
	Window window;

	static Display *display;
	static int screen;
	static int windows; /* reference counter to open windows */
};

#endif /* _XWINDOW_H_ */
