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

#include	<sgtty.h>

#define	NOBUF	512			/* Output buffer size.		*/

char	obuf[NOBUF];			/* Output buffer.		*/
int	nobuf;
struct	sgttyb	oldtty;			/* V6/V7 stty data.		*/
struct	sgttyb	newtty;
struct	tchars	oldtchars;		/* V7 editing.			*/
struct	tchars	newtchars;
struct	ltchars oldltchars;		/* 4.2 BSD editing.		*/
struct	ltchars	newltchars;
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
	register char	*cp;
	extern char	*getenv();

	if (ioctl(0, TIOCGETP, &oldtty) < 0)
		abort();
	newtty.sg_ospeed = oldtty.sg_ospeed;
	newtty.sg_ispeed = oldtty.sg_ispeed;
	newtty.sg_erase  = oldtty.sg_erase;
	newtty.sg_kill   = oldtty.sg_kill;
	newtty.sg_flags  = oldtty.sg_flags;
	newtty.sg_flags &= ~(ECHO|CRMOD);	/* Kill echo, CR=>NL.	*/
	newtty.sg_flags |= CBREAK;		/* Half-cooked mode.	*/
	if (ioctl(0, TIOCSETP, &newtty) < 0)
		abort();
	if (ioctl(0, TIOCGETC, &oldtchars) < 0)
		abort();
	newtchars.t_intrc  = 0xFF;		/* Interrupt.		*/
	newtchars.t_quitc  = 0xFF;		/* Quit.		*/
	newtchars.t_startc = 0x11;		/* ^Q, for terminal.	*/
	newtchars.t_stopc  = 0x13;		/* ^S, for terminal.	*/
	newtchars.t_eofc   = 0xFF;
	newtchars.t_brkc   = 0xFF;
	if (ioctl(0, TIOCSETC, &newtchars) < 0)
		abort();
	if (ioctl(0, TIOCGLTC, &oldltchars) < 0)
		abort();
	newltchars.t_suspc  = 0xFF;		/* Suspend #1.		*/
	newltchars.t_dsuspc = 0xFF;		/* Suspend #2.		*/
	newltchars.t_rprntc = 0xFF;
	newltchars.t_flushc = 0xFF;		/* Output flush.	*/
	newltchars.t_werasc = 0xFF;
	newltchars.t_lnextc = 0xFF;		/* Literal next.	*/
	if (ioctl(0, TIOCSLTC, &newltchars) < 0)
		abort();
	if ((cp=getenv("TERMCAP")) == NULL
	|| (nrow=getvalue(cp, "li")) <= 0
	|| (ncol=getvalue(cp, "co")) <= 0) {
		nrow = 24;
		ncol = 80;
	}
	if (nrow > NROW)			/* Don't crash if the	*/
		nrow = NROW;			/* termcap entry is	*/
	if (ncol > NCOL)			/* too big.		*/
		ncol = NCOL;
}

/*
 * This routine scans a string, which is
 * actually the return value of a getenv call for the TERMCAP
 * variable, looking for numeric parameter "name". Return the value
 * if found. Return -1 if not there. Assume that "name" is 2
 * characters long. This limited use of the TERMCAP lets us find
 * out the size of a window on the X display.
 */
getvalue(cp, name)
register char	*cp;
register char	*name;
{
	for (;;) {
		while (*cp!=0 && *cp!=':')
			++cp;
		if (*cp++ == 0)			/* Not found.		*/
			return (-1);
		if (cp[0]==name[0] && cp[1]==name[1] && cp[2]=='#')
			return (atoi(cp+3));	/* Stops on ":".	*/
	}
}

/*
 * This function gets called just
 * before we go back home to the shell. Put all of
 * the terminal parameters back.
 */
ttclose()
{
	ttflush();
	if (ioctl(0, TIOCSLTC, &oldltchars) < 0)
		abort();
	if (ioctl(0, TIOCSETC, &oldtchars) < 0)
		abort();
	if (ioctl(0, TIOCSETP, &oldtty) < 0)
		abort();
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
