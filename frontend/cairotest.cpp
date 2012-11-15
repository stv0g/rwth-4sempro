#include "XWindow.h"
#include "Plot.h"

#include <iostream>

#include <unistd.h>
#include <stdlib.h>

using namespace Cairo;

int main(int argc, char *argv[]) {
	XWindow::connect(":0"); // TODO parse from argv

	XEvent e;
	Color blue = { 0, 0, 1 };
	Color red = { 1, 0, 0 };
	Plot testPlot(800, 400);
	Plot testPlot2(800, 400);

	PlotSeries *demo1 = new PlotSeries(PlotSeries::STYLE_LINE, blue);
	PlotSeries *demo2 = new PlotSeries(PlotSeries::STYLE_LINE, red);
	testPlot.series.push_back(demo1);
	testPlot2.series.push_back(demo2);

	double phi = 0;
	double last = 0;

	while (1) {
		phi += 1e-1;

		last += -10 + rand()%21;

		demo2->push_back(last);

		if (demo2->size() > 400) demo2->pop_front();

		demo1->clear();
		for (int i = 0; i < 300; i++) {
			demo1->push_back(0.7*sin(i*phi/1e3)*sin(i * (M_PI/30.0) + phi));
		}

		usleep(0.2*1e5);
		testPlot.draw();
		testPlot2.draw();
	}
}
