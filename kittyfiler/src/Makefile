# GNU Makefile
CPP		=	clang++
CFLAGS		=	-std=c++17 -Wall
LFLAGS		=	-lc -ljsoncpp
APP		=	kittyfiler
.PHONY: all clean

%.o: %.cpp
	$(CPP) -c ${CFLAGS} -c -o $@ $<

$(APP): kittyfiler.o
	$(CPP) ${CFLAGS} ${LFLAGS} -o $@ $<

all: $(APP)

clean:
	rm -f *.o ${APP}
