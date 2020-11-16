/*
 * Windows terminal I/O. Based heavily on sys/unix/ttyio.c.
 * 
 * Jörgen Sigvardsson <jorgen.sigvardsson@gmail.com>
 */
#include    <Windows.h>
#include    <wincon.h>
#include	"def.h"

static char* obuf = NULL;
static int	nobuf;
int	nrow = NROW;				/* Terminal size, rows.		*/
int	ncol = NCOL;				/* Terminal size, columns.	*/
#define BUFLEN (nrow * ncol)
#define CSI "\x1b["

static DWORD old_output_mode;
static DWORD old_input_mode;
static CONSOLE_SCREEN_BUFFER_INFOEX oldtty;
static HANDLE consoleOutputHandle = INVALID_HANDLE_VALUE;
static HANDLE consoleInputHandle = INVALID_HANDLE_VALUE;

/*
 * This function gets called once, to set up
 * the terminal channel. On Ultrix is's tricky, since
 * we want flow control, but we don't want any characters
 * stolen to send signals. Use CBREAK mode, and set all
 * characters but start and stop to 0xFF.
 */
void ttopen()
{
    DWORD new_input_mode;
    DWORD new_output_mode;
    CONSOLE_SCREEN_BUFFER_INFOEX newtty;

	/* Open alternate screen buffer */
    printf(CSI "?1049h");
	
    consoleOutputHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    consoleInputHandle = GetStdHandle(STD_INPUT_HANDLE);
    if (consoleOutputHandle == INVALID_HANDLE_VALUE || consoleInputHandle == INVALID_HANDLE_VALUE)
        abort();

    if (!GetConsoleMode(consoleOutputHandle, &old_output_mode))
        abort();

    if (!GetConsoleMode(consoleInputHandle, &old_input_mode))
        abort();

    oldtty.cbSize = sizeof(oldtty);
    if (!GetConsoleScreenBufferInfoEx(consoleOutputHandle, &oldtty))
        abort();

    newtty = oldtty;
    newtty.dwSize.X = NCOL;
    newtty.dwSize.Y = NROW;
    newtty.srWindow.Top = 0;
    newtty.srWindow.Left = 0;
    newtty.srWindow.Bottom = NROW;
    newtty.srWindow.Right = NCOL;
    newtty.dwMaximumWindowSize.X = NCOL;
    newtty.dwMaximumWindowSize.Y = NROW;

    if (!SetConsoleScreenBufferInfoEx(consoleOutputHandle, &newtty))
        abort();

	new_input_mode = ENABLE_VIRTUAL_TERMINAL_INPUT;
    new_output_mode = old_output_mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING;

    if (!SetConsoleMode(consoleOutputHandle, new_output_mode))
        abort();

    if (!SetConsoleMode(consoleInputHandle, new_input_mode))
        abort();

    obuf = malloc(BUFLEN);
}

/*
 * This function gets called just
 * before we go back home to the shell. Put all of
 * the terminal parameters back.
 */
void ttclose()
{
    if (consoleOutputHandle != INVALID_HANDLE_VALUE) {
        SetConsoleScreenBufferInfoEx(consoleOutputHandle, &oldtty);
    	
        /* Exit the alternate buffer */
        printf(CSI "?1049l");

        SetConsoleMode(consoleOutputHandle, old_output_mode);
        SetConsoleMode(consoleInputHandle, old_input_mode);
        free(obuf);
    }	
}

/*
 * Flush output.
 */
void ttflush()
{
    if (nobuf != 0) {
        WriteConsoleA(consoleOutputHandle, obuf, nobuf, NULL, NULL);
        nobuf = 0;
    }
}


/*
 * Write character to the display.
 * Characters are buffered up, to make things
 * a little bit more efficient.
 */
void ttputc(char c) {
    if (nobuf >= BUFLEN)
        ttflush();
    obuf[nobuf++] = c;
}

/*
 * Read character from terminal.
 * All 8 bits are returned, so that you can use
 * a multi-national terminal.
 */
char ttgetc()
{
    INPUT_RECORD input;
    DWORD numEventsRead;

    for (;;) {
	    if (!ReadConsoleInput(consoleInputHandle, &input, 1, &numEventsRead))
            abort();
        if (input.EventType == KEY_EVENT && input.Event.KeyEvent.uChar.AsciiChar != 0 && input.Event.KeyEvent.bKeyDown)
			return input.Event.KeyEvent.uChar.AsciiChar;
    }
}
