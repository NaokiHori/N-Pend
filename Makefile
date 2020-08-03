CC       := clang++
CFLAGS   := -std=c++14 -O3
INCLUDES := -Iinclude -I/usr/X11R6/include -I/usr/X11R6/include/X11
LIBS     := -L/usr/X11R6/lib -L/usr/X11R6/lib/X11 -lX11 -lm
SRCDIR   := src
OBJDIR   := obj
TARGET   := a.out


all: $(TARGET)

$(TARGET): $(OBJDIR)/Pendulum.o $(OBJDIR)/Visualizer.o $(OBJDIR)/main.o
	$(CC) $(CFLAGS) $(INCLUDES) -o $@ $^ $(LIBS)

$(OBJDIR)/Pendulum.o: $(SRCDIR)/Pendulum.cc
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OBJDIR)/Visualizer.o: $(SRCDIR)/Visualizer.cc
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OBJDIR)/main.o: $(SRCDIR)/main.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	$(RM) $(TARGET) $(OBJDIR)/*.o
