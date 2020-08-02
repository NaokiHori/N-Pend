
SRC := Pendulum.cc main.cpp

all:
	clang++ -I/usr/X11R6/include -I/usr/X11R6/include/X11 $(SRC) -L/usr/X11R6/lib -L/usr/X11R6/lib/X11 -lX11 -lm
