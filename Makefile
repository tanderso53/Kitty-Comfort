# Makefile for building, checking, and uploading
# Arduino Sketches
#
# Licensed under GPL v3

# Paths for cool people
ROOTDIR		=	$(.CURDIR)
SKETCHES	=	${ROOTDIR}/sketches
LIBDIR		=	${ROOTDIR}/libraries

# Compiler settings
CC		=	arduino-cli
CFLAGS		=	-b arduino:avr:uno

# Main application src
APP		=	kitycomfort
INOFILE		=	${SKETCHES}/${APP}.ino

# Target to compile the sketch

(.Phony): all flycheck

flycheck:
	$(CC) compile $(CFLAGS) $(INOFILE)

all: flycheck
