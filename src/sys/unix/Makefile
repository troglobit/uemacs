VPATH	= ../..:../../tty/ansi
EXEC	= uemacs
OBJS	= basic.o buffer.o cinfo.o display.o echo.o extend.o file.o	\
	  	kbd.o line.o main.o random.o region.o search.o	\
	  	symbol.o version.o window.o word.o

# front end: ansi
OBJS	+= tty.o ttykbd.o

# back end: unix
OBJS	+= fileio.o spawn.o ttyio.o

CFLAGS	+= -I. -I../../tty/ansi -I../.. -std=c89

all:	$(EXEC)

$(EXEC): $(OBJS)

clean:
	$(RM) $(OBJS)