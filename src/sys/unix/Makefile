# Makefile for MicroEMACS.
# Is there a better way to do the rebuilds, other than using
# the links?

SYS	= unix
TTY	= ansi

VPATH	= tty/$(TTY):sys/$(SYS)
CPPFLAGS= -I. -Itty/$(TTY) -Isys/$(SYS)
CFLAGS	= -O -g -std=c89
CFLAGS += -Wno-incompatible-pointer-types -Wno-builtin-declaration-mismatch

OBJ =	basic.o \
	buffer.o \
	cinfo.o \
	display.o \
	echo.o \
	extend.o \
	file.o \
	kbd.o \
	line.o \
	main.o \
	random.o \
	region.o \
	search.o \
	symbol.o \
	version.o \
	window.o \
	word.o \
	fileio.o \
	spawn.o \
	ttyio.o \
	tty.o \
	ttykbd.o

OBJS += tty.o ttykbd.o \
	spawn.o fileio.o ttyio.o

xemacs:		$(OBJ)
	cc -o xemacs $(OBJ)

$(OBJ):		def.h sysdef.h ttydef.h

clean:
	rm -f xemacs $(OBJ)

distclean: clean
	rm *~
