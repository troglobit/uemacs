# Makefile for MicroEMACS.
# Is there a better way to do the rebuilds, other than using
# the links?

SYS	= ultrix
TTY	= ansi

CFLAGS	= -O

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

xemacs:		$(OBJ)
	cc -o xemacs $(OBJ)

$(OBJ):		def.h sysdef.h ttydef.h

sysdef.h:	sys/$(SYS)/sysdef.h	# Update links, if needed.
	ln sys/$(SYS)/sysdef.h .

ttydef.h:	tty/$(TTY)/ttydef.h
	ln tty/$(TTY)/ttydef.h .

fileio.c:	sys/$(SYS)/fileio.c
	ln sys/$(SYS)/fileio.c .

spawn.c:	sys/$(SYS)/spawn.c
	ln sys/$(SYS)/spawn.c .

tty.c:		tty/$(TTY)/tty.c
	ln tty/$(TTY)/tty.c .

ttyio.c:	sys/$(SYS)/ttyio.c
	ln sys/$(SYS)/ttyio.c .

ttykbd.c:	tty/$(TTY)/ttykbd.c
	ln tty/$(TTY)/ttykbd.c .
