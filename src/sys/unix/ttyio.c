/*
 * Name:	MicroEMACS
 *		Ultrix-32 terminal I/O.
 * Version:	29
 * Last edit:	05-Feb-86
 * By:		rex::conroy
 *		decvax!decwrl!dec-rhea!dec-rex!conroy
 *
 * The functions in this file
 * negotiate with the operating system for
 * keyboard characters, and write characters to
 * the display in a barely buffered fashion.
 */
#include	"def.h"

#include	<termios.h>
#include	<poll.h>

#define	NOBUF	512			/* Output buffer size.		*/

char	obuf[NOBUF];			/* Output buffer.		*/
int	nobuf;
struct	termios	oldtty;			/* POSIX tty settings. */
struct	termios	newtty;
int	nrow;				/* Terminal size, rows.		*/
int	ncol;				/* Terminal size, columns.	*/

/*
 * This function gets called once, to set up
 * the terminal channel. On Ultrix is's tricky, since
 * we want flow control, but we don't want any characters
 * stolen to send signals. Use CBREAK mode, and set all
 * characters but start and stop to 0xFF.
 */
ttopen()
{
	/* Save pos+attr, disable margins, set cursor far away, query pos */
	const char query[] = "\e7" "\e[r" "\e[999;999H" "\e[6n";
	struct pollfd fd = { 1, POLLIN, 0 };
	int row, col;

	/* Adjust output channel */
	tcgetattr(1, &oldtty);			/* save old state */
	newtty = oldtty;			/* get base of new state */
	cfmakeraw(&newtty);
	tcsetattr(1, TCSADRAIN, &newtty);	/* set mode */

	/* Query size of terminal by first trying to position cursor */
	if (write(1, query, sizeof(query)) != -1 && poll(&fd, 1, 300) > 0) {
		/* Terminal responds with \e[row;posR */
		if (scanf("\e[%d;%dR", &nrow, &ncol) != 2) {
			nrow = 80;
			ncol = 24;
		}
	}

	if (nrow > NROW)
		nrow = NROW;
	if (ncol > NCOL)
		ncol = NCOL;
}

/*
 * This function gets called just
 * before we go back home to the shell. Put all of
 * the terminal parameters back.
 */
ttclose()
{
	ttflush();
	tcsetattr(1, TCSADRAIN, &oldtty);	/* return to original mode */
}

/*
 * Write character to the display.
 * Characters are buffered up, to make things
 * a little bit more efficient.
 */
ttputc(c)
{
	if (nobuf >= NOBUF)
		ttflush();
	obuf[nobuf++] = c;
}

/*
 * Flush output.
 */
ttflush()
{
	if (nobuf != 0) {
		write(1, obuf, nobuf);
		nobuf = 0;
	}
}

/*
 * Read character from terminal.
 * All 8 bits are returned, so that you can use
 * a multi-national terminal.
 */
ttgetc()
{
	char	buf[1];

	while (read(0, &buf[0], 1) != 1)
		;
	return (buf[0] & 0xFF);
}
