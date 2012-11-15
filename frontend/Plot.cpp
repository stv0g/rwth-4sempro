#include <math.h>
#include <float.h>

#include <iostream>

#include "Plot.h"

PlotSeries::PlotSeries(PlotSeries::Style style, Color color)
  : color(color), style(style)
{ }

void PlotSeries::draw(RefPtr<Context> ctx) {
	ctx->set_source_rgb(color.red, color.green, color.blue); /* set series color */

	/* determine maxima and minima for autoscale */
	double min = DBL_MAX, max = DBL_MIN;
	for (std::list<double>::iterator it = begin(); it != end(); it++) {
		if (*it > max) max = *it;
		if (*it < min) min = *it;
	}

	/* stroke */

	int width = 800;//dynamic_cast<RefPtr<XlibSurface>>(ctx->get_target())->get_width();
	int height = 400;//dynamic_cast<RefPtr<XlibSurface>>(ctx->get_target())->get_height();

	ctx->move_to(Plot::PADDING, (height/2) + (height-2*Plot::PADDING) * (front()/max));

	int i = 0;
	for (std::list<double>::iterator it = begin(); it != end(); it++) {
		i++;
		ctx->line_to(i * 2 + Plot::PADDING, (height/2) + (height-2*Plot::PADDING) * (*it/max*0.5));
	}
	ctx->stroke();
}

Plot::Plot(int width, int height)
  : width(width), height(height)
{
	window = XWindow::create("Frontend", 1, 1, width, height);
	surface = XlibSurface::create(window->getDisplay(), window->getWindow(), window->getVisual(), width, height);
}

void Plot::draw() {
	RefPtr<Context> ctx = Context::create(surface);
	ctx->set_antialias(ANTIALIAS_SUBPIXEL);

	/* background */
	ctx->set_source_rgb(0, 0, 0); /* black */
	ctx->paint();

	ctx->set_source_rgb(0, 0.7, 0.1); /* axis & tick color */
	drawAxes(ctx);
	drawTicks(ctx);

	for (std::list<PlotSeries *>::iterator it = series.begin(); it != series.end(); it++) {
		(*it)->draw(ctx);
	}
}

void Plot::drawAxes(RefPtr<Context> ctx) {
	ctx->set_line_width(1.8);

	/* x-axis */
	ctx->move_to(PADDING, height-PADDING);
	ctx->line_to(width-PADDING, height-PADDING);
	ctx->stroke();

	/* y-axis */
	ctx->move_to(PADDING, height-PADDING);
	ctx->line_to(PADDING, PADDING);
	ctx->stroke();

	/* arrows (arcs) */
	ctx->move_to(width-PADDING-2, height-PADDING-4);
	ctx->line_to(width-PADDING+5, height-PADDING);
	ctx->line_to(width-PADDING-2, height-PADDING+4);
	ctx->stroke();

	ctx->move_to(PADDING-4, PADDING+2);
	ctx->line_to(PADDING, PADDING-5);
	ctx->line_to(PADDING+4, PADDING+2);
	ctx->stroke();
}

void Plot::drawTicks(RefPtr<Context> ctx) {
	int ticks = 20;
	int intv_x = (width-2*PADDING)/ticks;
	int intv_y = (height-2*PADDING)/ticks;

	ctx->set_line_width(1);

	for (int i = 0; i < ticks; i++) {
		/* x-axis */
		ctx->move_to(PADDING + i*intv_x, height-PADDING-3);
		ctx->line_to(PADDING + i*intv_x, height-PADDING+3);
		ctx->stroke();

		/* y-axis */
		ctx->move_to(PADDING-3, PADDING + (i+1)*intv_y);
		ctx->line_to(PADDING+3, PADDING + (i+1)*intv_y);
		ctx->stroke();
	}
}

Plot::~Plot() {

}
