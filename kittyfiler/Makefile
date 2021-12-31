#####################################################################
#                                                                   #
#                            KITTYFILER                             #
#                  A program to file cat data into                  #
#                       a Postgresql database                       #
#                                                                   #
#####################################################################

# Copyright 2021 Tyler J. Anderson

# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:

# 1. Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.

# 2. Redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in the
# documentation and/or other materials provided with the distribution.

# 3. Neither the name of the copyright holder nor the names of its
# contributors may be used to endorse or promote products derived from
# this software without specific prior written permission.

# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.

# Kittyfiler Makefile

srcdir		=	./src
OBJDIR		:=	./objdir

# BUILD SECTION
CC		=	clang
CXX		=	clang++
CFLAGS		+=	-Wall -I/usr/local/include
CXXFLAGS	+=	-std=c++17
LDLIBS		+=	-lc -ljsoncpp -lpqxx
#ifdef $(FREEBSD)
LDLIBS		+=	-lpq
#endif
LDFLAGS		+=	-L/usr/local/lib
APP		=	kittyfiler
C_SRCS		=	remote.c
CXX_SRCS	=	kittyfiler.cpp connection.cpp database.cpp \
			cli.cpp app.cpp jsonparse.cpp
C_OBJS		=	$(addprefix $(OBJDIR)/,$(C_SRCS:.c=.o))
CXX_OBJS	=	$(addprefix $(OBJDIR)/,$(CXX_SRCS:.cpp=.o))
OBJS		:=	$(C_OBJS) $(CXX_OBJS)
HPP		=	connection.hpp database.hpp handler.hpp app.hpp
HPP		+=	cli.hpp jsonparse.h
LICENSE		=	../../LICENSE
.PHONY: all clean install

$(OBJDIR)/%.o: $(srcdir)/%.c $(addprefix $(srcdir)/,$(HPP))
	@echo "*** BUILDING $@ ***"
	$(CC) ${CFLAGS} -c -o $@ $<

$(OBJDIR)/%.o: $(srcdir)/%.cpp $(addprefix $(srcdir)/,$(HPP))
	@echo "*** BUILDING $@ ***"
	$(CXX) ${CFLAGS} $(CXXFLAGS) -c -o $@ $<

$(APP): $(OBJS)
	@echo "*** BUILDING $@ ***"
	$(CXX) ${CFLAGS} ${LDFLAGS} ${LDLIBS} -o $@ $(OBJS)
	@echo "Complete! Install with \"make install\""

all: $(APP)

clean:
	$(RM) $(APP)
	$(RM) -R $(OBJDIR)

$(OBJS): | $(OBJDIR)

$(OBJDIR):
	mkdir $(OBJDIR)

# INSTALL SECTION
INSTALL=install
INSTALL_PROGRAM=$(INSTALL)
INSTALL_DATA=$(INSTALL) -m 644
PREFIX=/usr/local
BINDIR=$(PREFIX)/bin
DATAROOTDIR=$(PREFIX)/share
DATADIR=$(DATAROOTDIR)
SYSCONFDIR=$(PREFIX)/etc
DOCDIR=$(DATAROOTDIR)/doc/$(APP)

install: all
	$(INSTALL_PROGRAM) $(APP) $(DESTDIR)$(BINDIR)/$(APP)
	$(INSTALL_DATA) $(LICENSE) $(DESTDIR)$(DATADIR)/$(APP)/LICENSE
