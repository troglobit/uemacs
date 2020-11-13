# Makefile for MicroEMACS.
# Is there a better way to do the rebuilds, other than using
# the links?

SYS	= unix
TTY	= ansi

VPATH	= tty/$(TTY):sys/$(SYS)
CPPFLAGS= -I. -Itty/$(TTY) -Isys/$(SYS)
CFLAGS	= -O -g -std=c89
CFLAGS += -Wno-incompatible-pointer-types -Wno-builtin-declaration-mismatch
OBJ_DIR = ../obj
BIN_DIR = ../bin
TARGET  = $(BIN_DIR)/xemacs
OBJ     = $(addprefix $(OBJ_DIR)/, \
	basic.o \
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
	ttykbd.o \
)

$(TARGET): $(OBJ_DIR) $(OBJ)
	mkdir -p $(BIN_DIR)
	cc -o $@ $(OBJ)

$(OBJ):	def.h sysdef.h ttydef.h

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS) $(CPPFLAGS)

clean:
	rm -rf $(TARGET) $(OBJ_DIR)

distclean: clean
	rm *~

