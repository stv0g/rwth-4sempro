# adding cairo.pc
export PKG_CONFIG_PATH:=/usr/lib/x86_64-linux-gnu/pkgconfig/

# programs
CC = gcc
PC=pkg-config
RM=rm

TARGET=frontend
OBJS=Plot.o XWindow.o cairotest.o

CFLAGS = -Wall `$(PC) --cflags cairomm-xlib-1.0`
LIBS = -lm `$(PC) --libs cairomm-xlib-1.0`
INC = -I/usr/include/cairomm-1.0/

all: $(OBJS)
	$(CC) $(OBJS) $(LIBS) -o $(TARGET)

%.o: %.cpp
	$(CC) $(CFLAGS) $(INC) -c -o $@ $<

clean:
	$(RM) frontend
	$(RM) $(OBJS)
