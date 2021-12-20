#####################################################################
#                                                                   #
#                           KITTYCOMFORT                            #
#                 Microcontroller project to maintain               #
#                      optimal housecat comfort                     #
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

# Makefile for building, checking, and uploading
# Arduino Sketches
#

srcdir		=	.

# Arduino config
ALOCALDIR	=	$(HOME)/.arduino15
APKGDIR		=	$(ALOCALDIR)/packages
ALIBPATH	=	$(HOME)/Arduino/libraries

# Compiler settings
ARDUINO		=	arduino-builder
AFLAGS		=	-hardware $(APKGDIR)/arduino/hardware \
			-tools $(APKGDIR)/arduino/tools \
			-libraries $(ALIBPATH)
AFQBN		=	samd:1.8.11:nano_33_iot
builddir	:=	./build

# Main application src
APP		=	kittycomfort
ASRCS		=	${APP}.ino
AEXTRASRCS	=	arduino_secrets.h ammoniasensor.cpp ammoniasensor.h
ELFS		:=	$(addprefix $(builddir)/, $(ASRCS:.ino=.elf))

# Target to compile the sketch

.PHONY: all install clean

all:

$(APP): $(ELFS)
	touch $(APP)

$(buildir)/%.elf: $(addprefix $(srcdir)/, %.ino $(AEXTRASRCS))
	@echo "*** Building $@ ***"
	$(ARDUINO) -compile -fqbn $(AFQBN) $(AFLAGS) \
		-build-path $(builddir) $<

$(ELFS): | $(builddir)

$(builddir):
	mkdir -p $(builddir)

install:
	@echo "Install for sketches not yet implemented"

clean:
	$(RM) -R $(builddir)
	$(RM) $(APP)
