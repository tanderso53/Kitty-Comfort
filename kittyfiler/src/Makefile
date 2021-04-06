# GNU Makefile
CPP		=	clang++
CFLAGS		=	-std=c++17 -Wall
LFLAGS		=	-lc -ljsoncpp -lpqxx
APP		=	kittyfiler
OBJ		=	kittyfiler.o connection.o database.o cli.o
HPP		=	connection.hpp database.hpp handler.hpp
HPP		+=	cli.hpp
.PHONY: all clean

%.o: %.cpp
	$(CPP) -c ${CFLAGS} -c -o $@ $<

$(APP): $(OBJ) $(HPP)
	$(CPP) ${CFLAGS} ${LFLAGS} -o $@ $(OBJ)

all: $(APP)

clean:
	rm -f *.o ${APP}
