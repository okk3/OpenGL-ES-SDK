BIN=bin/GLESSample
OBJS=main.o nativewin_x11.o
INCLUDES=-I../include
LIBS=-lX11 -lEGL -lGLESv2
CC=g++
CCFLAGS=-Wall -O0 -ggdb2 -fno-exceptions -DNDEBUG $(INCLUDES)
LD=g++
LDFLAGS=-L../x86 $(LIBS)

$(BIN): $(OBJS)
	$(LD) $(OBJS) $(LDFLAGS) -o $@

%.o : %.cpp
	$(CC) $(CCFLAGS) -c $< -o $@

all: $(BIN)

clean:
	rm -rf $(OBJS) $(BIN)

