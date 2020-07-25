/*
 * Name:	MicroEMACS
 *		Digital ANSI terminal display
 * Version:	29
 * Last edit:	10-Feb-86
 * By:		rex::conroy
 *		decvax!decwrl!dec-rhea!dec-rex!conroy
 *
 * The SCALD display is just an ANSI display, with
 * some special hacks to kludge around the bugs, and
 * to make it a bit more friendly. The support is
 * unquestionably non-optimal. The costs are wrong; in
 * fact, display should be fixed up to understand non
 * linear cost devices like the SCALD. The BackIndex
 * sequence used in the insert line is defective in
 * the display firmware, so we set the cost high to
 * discourage its use. Perhaps the cost should be
 * set to infinity!
 */
#include	"def.h"

#define	SCALD	0			/* Buggy display.		*/

#define	BEL	0x07			/* BEL character.		*/
#define	ESC	0x1B			/* ESC character.		*/
#define	LF	0x0A			/* Line feed.			*/

extern	int	ttrow;
extern	int	ttcol;
extern	int	tttop;
extern	int	ttbot;
extern	int	tthue;

#if	SCALD

int	tceeol	=	3;		/* Costs, SCALDstation.		*/
int	tcinsl	= 	100;
int	tcdell	=	100;

#else

int	tceeol	=	3;		/* Costs, ANSI display.		*/
int	tcinsl	= 	17;
int	tcdell	=	16;

#endif

/*
 * Initialize the terminal when the editor
 * gets started up. This is a no-op on the ANSI
 * display. On the SCALD display, it turns off the
 * half-screen scroll, because this appears to really
 * confuse the scrolling region firmware in the
 * display.
 */
ttinit()
{
#if	SCALD
	ttputc(ESC);			/* Cancel jump interval.	*/
	ttputc('[');
	asciiparm(1);
	ttputc('j');
#endif
}

/*
 * Clean up the terminal, in anticipation of
 * a return to the command interpreter. This is a no-op
 * on the ANSI display. On the SCALD display, it sets the
 * window back to half screen scrolling. Perhaps it should
 * query the display for the increment, and put it
 * back to what it was.
 */
tttidy()
{
#if	SCALD
	ttputc(ESC);			/* Half screen.			*/
	ttputc('[');
	asciiparm(nrow/2);
	ttputc('j');
#endif
}

/*
 * Move the cursor to the specified
 * origin 0 row and column position. Try to
 * optimize out extra moves; redisplay may
 * have left the cursor in the right
 * location last time!
 */
ttmove(row, col)
{
	if (ttrow!=row || ttcol!=col) {
		ttputc(ESC);
		ttputc('[');
		asciiparm(row+1);
		ttputc(';');
		asciiparm(col+1);
		ttputc('H');
		ttrow = row;
		ttcol = col;
	}
}

/*
 * Erase to end of line.
 */
tteeol()
{
	ttputc(ESC);
	ttputc('[');
	ttputc('K');
}

/*
 * Erase to end of page.
 */
tteeop()
{
	ttputc(ESC);
	ttputc('[');
	ttputc('J');
}

/*
 * Make a noise.
 */
ttbeep()
{
	ttputc(BEL);
	ttflush();
}

/*
 * Convert a number to decimal
 * ascii, and write it out. Used to
 * deal with numeric arguments.
 */
asciiparm(n)
register int	n;
{
	register int	q;

	q = n/10;
	if (q != 0)
		asciiparm(q);
	ttputc((n%10) + '0');
}

/*
 * Insert a block of blank lines onto the
 * screen, using a scrolling region that starts at row
 * "row" and extends down to row "bot". Deal with the one
 * line case, which is a little bit special, with special
 * case code. Put all of the back index commands out
 * in a block. The SCALDstation loses the position
 * of the cursor.
 */
ttinsl(row, bot, nchunk)
{
	if (row == bot) {			/* Funny case.		*/
		if (nchunk != 1)
			abort();
		ttmove(row, 0);
		tteeol();
	} else {				/* General case.	*/
		ttwindow(row, bot);
		ttmove(row, 0);
		while (nchunk--) {
			ttputc(ESC);		/* Back index.		*/
			ttputc('M');
		}
#if	SCALD
		ttrow = HUGE;
		ttcol = HUGE;
#endif
	}
}

/*
 * Delete a block of lines, with the uppermost
 * line at row "row", in a screen slice that extends to
 * row "bot". The "nchunk" is the number of lines that have
 * to be deleted. Watch for the pathalogical 1 line case,
 * where the scroll region is *not* the way to do it.
 * The block delete is used by the slightly more
 * optimal display code.
 */
ttdell(row, bot, nchunk)
{
	if (row == bot) {			/* Funny case.		*/
		if (nchunk != 1)
			abort();
		ttmove(row, 0);
		tteeol();
	} else {				/* General case.	*/
		ttwindow(row, bot);
		ttmove(bot, 0);
		while (nchunk--)
			ttputc(LF);
#if	SCALD
		ttrow = HUGE;
		ttcol = HUGE;
#endif
	}
}

/*
 * This routine sets the scrolling window
 * on the display to go from line "top" to line
 * "bot" (origin 0, inclusive). The caller checks
 * for the pathalogical 1 line scroll window that
 * doesn't work right, and avoids it. The "ttrow"
 * and "ttcol" variables are set to a crazy value
 * to ensure that the next call to "ttmove" does
 * not turn into a no-op (the window adjustment
 * moves the cursor).
 */
ttwindow(top, bot)
{
	if (tttop!=top || ttbot!=bot) {
		ttputc(ESC);
		ttputc('[');
		asciiparm(top+1);
		ttputc(';');
		asciiparm(bot+1);
		ttputc('r');
		ttrow = HUGE;			/* Unknown.		*/
		ttcol = HUGE;
		tttop = top;			/* Remember region.	*/
		ttbot = bot;
	}
}

/*
 * Switch to full screen scroll. This is
 * used by "spawn.c" just before is suspends the
 * editor, and by "display.c" when it is getting ready
 * to exit. This function gets to full screen scroll
 * by sending a DECSTBM with default parameters, but
 * I think that this is wrong. The SRM seems to say
 * that the default for Pb is 24, not the size of the
 * screen, which seems really dumb. Do I really have
 * to read the size of the screen as in "ttresize"
 * to do this right?
 */
ttnowindow()
{
	ttputc(ESC);
	ttputc('[');
	ttputc(';');
	ttputc('r');
	ttrow = HUGE;				/* Unknown.		*/
	ttcol = HUGE;
	tttop = HUGE;				/* No scroll region.	*/
	ttbot = HUGE;
}

/*
 * Set the current writing color to the
 * specified color. Watch for color changes that are
 * not going to do anything (the color is already right)
 * and don't send anything to the display.
 * The rainbow version does this in putline.s on a
 * line by line basis, so don't bother sending
 * out the color shift.
 */
ttcolor(color)
register int	color;
{
	if (color != tthue) {
/*
if	!RAINBOW
*/
		if (color == CTEXT) {		/* Normal video.	*/
			ttputc(ESC);
			ttputc('[');
			ttputc('m');
		} else if (color == CMODE) {	/* Reverse video.	*/
			ttputc(ESC);
			ttputc('[');
			ttputc('7');
			ttputc('m');
		}
/*
endif
*/
		tthue = color;			/* Save the color.	*/
	}
}

/*
 * This routine is called by the
 * "refresh the screen" command to try and resize
 * the display. The new size, which must be deadstopped
 * to not exceed the NROW and NCOL limits, it stored
 * back into "nrow" and "ncol". Display can always deal
 * with a screen NROW by NCOL. Look in "window.c" to
 * see how the caller deals with a change.
 */
ttresize()
{
	register int	c;
	register int	newnrow;
	register int	newncol;

	ttputc(ESC);				/* Off the end of the	*/
	ttputc('[');				/* world. The terminal	*/
	asciiparm(HUGE);			/* will chop it.	*/
	ttputc(';');
	asciiparm(HUGE);
	ttputc('H');
	ttrow = HUGE;				/* Unknown.		*/
	ttcol = HUGE;
	ttputc(ESC);				/* Report position.	*/
	ttputc('[');
	ttputc('6');
	ttputc('n');
	ttflush();
	if (ttgetc()!=ESC || ttgetc()!='[')
		return;
	newnrow = 0;
	while ((c=ttgetc())>='0' && c<='9')
		newnrow = 10*newnrow + c - '0';
	if (c != ';')
		return;
	newncol = 0;
	while ((c=ttgetc())>='0' && c<='9')
		newncol = 10*newncol + c - '0';
	if (c != 'R')
		return;
	if (newnrow < 1)			/* Check limits.	*/
		newnrow = 1;
	else if (newnrow > NROW)
		newnrow = NROW;
	if (newncol < 1)
		newncol = 1;
	else if (newncol > NCOL)
		newncol = NCOL;
	nrow = newnrow;
	ncol = newncol;
}
