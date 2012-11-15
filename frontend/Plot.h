#ifndef _PLOT_H_
#define _PLOT_H_

#include <list>
#include <cairomm/cairomm.h>
#include <cairomm/xlib_surface.h>

#include "XWindow.h"

using namespace Cairo;

struct Color {
	double red, green, blue, alpha;
};

class PlotSeries : public std::list<double> {

  public:
	Color color;
	enum Style { STYLE_LINE, STYLE_SPLINE, STYLE_SCATTER } style;

	PlotSeries(enum Style style, Color color);
	void draw(RefPtr<Context> ctx);
};

class Plot {

	friend class PlotSeries;

  public:
	Plot(int width = 400, int height = 300);
	virtual ~Plot();

	void draw();

	std::list<PlotSeries *> series;

  protected:
	RefPtr<XWindow> window;
	RefPtr<Surface> surface;

	int width, height;

	void drawAxes(RefPtr<Context> ctx);
	void drawTicks(RefPtr<Context> ctx);

	static const int PADDING = 20;
};

#endif /* _PLOT_H_ */
