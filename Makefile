
SRC := main.cpp

all:
	icpc -I/usr/X11R6/include -I/usr/X11R6/include/X11 $(SRC) -L/usr/X11R6/lib -L/usr/X11R6/lib/X11 -lX11
