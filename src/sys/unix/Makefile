# Makefile for MicroEMACS.

SYS	= unix
TTY	= ansi

VPATH	= tty/$(TTY):sys/$(SYS)

CPPFLAGS= -I. -Itty/$(TTY) -Isys/$(SYS)
CFLAGS	= -O -g -std=c89
CFLAGS += -Wno-incompatible-pointer-types -Wno-builtin-declaration-mismatch

OBJ     = basic.o \
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

xemacs: $(OBJ)
	$(CC) -o $@ $(OBJ)

$(OBJ):	def.h sysdef.h ttydef.h

clean:
	$(RM) *.o xemacs

distclean: clean
	$(RM) *~

