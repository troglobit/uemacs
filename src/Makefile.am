bin_PROGRAMS    = uemacs

# TODO: qualify based on target and front-end
uemacs_CPPFLAGS = -I$(srcdir) -I$(srcdir)/tty/ansi -I$(srcdir)/sys/unix
uemacs_CFLAGS   = -std=c89 -Wno-incompatible-pointer-types -Wno-return-type			\
		  -Wno-builtin-declaration-mismatch
uemacs_SOURCES  = basic.c buffer.c cinfo.c def.h display.c echo.c extend.c	\
		  file.c kbd.c line.c main.c random.c region.c search.c		\
		  symbol.c version.c window.c word.c

# TODO: qualify by figuring out target system
uemacs_SOURCES += sys/unix/fileio.c sys/unix/spawn.c sys/unix/sysdef.h sys/unix/ttyio.c

# TODO: qualify by detected front end
uemacs_SOURCES += tty/ansi/tty.c tty/ansi/ttydef.h tty/ansi/ttykbd.c
