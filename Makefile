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

# Top-level Makefile for Kittycomfort and Kittyfiler
#

.PHONY: all kittycomfort kittyfiler install-kittycomfort install-kittyfiler install clean

kcdir		=	./sketches/kittycomfort
kfdir		=	./kittyfiler

all: kittycomfort kittyfiler

kittycomfort:
	@echo "@@@@@@@@@@ BUILDING $@ @@@@@@@@@@"
	$(MAKE) -e -C $(kcdir)

kittyfiler:
	@echo "@@@@@@@@@@ BUILDING $@ @@@@@@@@@@"
	$(MAKE) -e -C $(kfdir)

install: install-kittycomfort install-kittyfiler

install-kittycomfort:
	@echo "@@@@@@@@@@ INSTALLING $@ @@@@@@@@@@"
	$(MAKE) -e -C $(kcdir) install

install-kittyfiler:
	@echo "@@@@@@@@@@ INSTALLING $@ @@@@@@@@@@"
	$(MAKE) -e -C $(kfdir) install

clean:
	@echo "@@@@@@@@@@ CLEANING KITTYCOMFORT @@@@@@@@@@"
	$(MAKE) -e -C $(kcdir) clean
	@echo "@@@@@@@@@@ CLEANING KITTYFILER @@@@@@@@@@"
	$(MAKE) -e -C $(kfdir) clean
