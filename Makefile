TARGET		= nanotter
OBJS_TARGET	= $(TARGET).o

CFLAGS = -O0 -g -std=gnu++11 -fpermissive
LDFLAGS =
LIBS = -ltwitcurl -lncurses -lstdc++ -ljson

include Makefile.in
