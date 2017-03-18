TARGET		= nanotter
OBJS_TARGET	= $(TARGET).o Buffer.o Editor.o tui.o timeline.o api.o

CFLAGS = -O0 -g -fpermissive
LDFLAGS =
LIBS = -ltwitcurl -lncursesw -lstdc++ -ljson -lpthread 

include Makefile.in
