/*
 * Name:	MicroEMACS
 *		Heath H19 display
 * Version:	29
 * Last edit:	10-Feb-86
 * By:		rex::conroy
 *		decvax!decwrl!dec-rhea!dec-rex!conroy
 *
 * This code simulates scrolling regions by using the
 * insert line and delete line functions. Should display
 * handling be told about this?
 * This is code is by Rich Ellison. A man always does
 * the support for the terminal he owns.
 */
#include	"def.h"

#define	BEL	0x07			/* BEL character.		*/
#define	ESC	0x1B			/* ESC character.		*/
#define	LF	0x0A			/* Line feed.			*/

extern	int	ttrow;
extern	int	ttcol;
extern	int	tttop;
extern	int	ttbot;
extern	int	tthue;

int	tceeol	=	2;		/* Costs.			*/
int	tcinsl	=	11;
int	tcdell	=	11;

/*
 * The Heath needs no initialization.
 */
ttinit()
{
}

/*
 * The Heath needs no tidy up.
 */
tttidy()
{
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
		if (row > NROW)
			row = NROW;
		if (col > NCOL)
			col = NCOL;
		ttputc(ESC);
		ttputc('Y');
		ttputc(row+' ');
		ttputc(col+' ');
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
	ttputc('K');
}

/*
 * Erase to end of page.
 */
tteeop()
{
	ttputc(ESC);
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
 * Insert nchunk blank line(s) onto the
 * screen, scrolling the last line on the
 * screen off the bottom. This is done with
 * a cluster of clever insert and delete commands,
 * because there are no scroll regions.
 */
ttinsl(row, bot, nchunk)
{
	register int	i;

	if (row == bot) {		/* Case of one line insert is 	*/
		ttmove(row, 0);		/*	special			*/
		tteeol();
		return;
	}
	ttmove(1+bot-nchunk, 0);
	for (i=0; i<nchunk; i++) {	/* For all lines in the chunk	*/
		ttputc(ESC);		/* DEL current line, anything	*/
		ttputc('M');		/* below moves up.		*/
	}
	ttmove(row, 0);
	for (i=0; i<nchunk; i++) {	/* For all lines in the chunk	*/
		ttputc(ESC);		/* INS before current line,	*/
		ttputc('L');		/* sliding stuff down.		*/
	}
	ttrow = row;			/* End up on current line	*/
	ttcol = 0;
}

/*
 * Delete nchunk line(s) "row", replacing the
 * bottom line on the screen with a blank
 * line. This is done with a crafty sequences
 * of insert and delete line; there is no scroll
 * region on the Heath. The presence of the
 * echo area makes a boundry condition
 * go away.
 */
ttdell(row, bot, nchunk)
{
	register int	i;

	if (row == bot) {		/* One line special case	*/
		ttmove(row, 0);
		tteeol();
		return;
	}
	ttmove(row, 0);
	for (i=0; i<nchunk; i++) {	/* For all lines in chunk	*/
		ttputc(ESC);		/* DEL top line, lines below	*/
		ttputc('M');		/* all move up.			*/
	}
	ttmove(1+bot-nchunk,0);
	for (i=0; i<nchunk; i++) {	/* For all lines in chunk	*/
		ttputc(ESC);		/* INS line before botton,	*/ 
		ttputc('L');		/* all lines move down.		*/ 
	}
	ttrow = bot-nchunk;
	ttcol = 0;
}

/*
 * No-op.
 */
ttwindow(top, bot)
{
}

/*
 * No-op.
 */
ttnowindow()
{
}

/*
 * Set display color on Heath. Normal
 * video is text color. Reverse video is used for
 * the mode line. Rich knew the sequences for the
 * Heath by heart.
 */
ttcolor(color)
register int	color;
{
	if (color != tthue) {
		if (color == CTEXT) {		/* Normal video.	*/
			ttputc(ESC);
			ttputc('q');
		} else if (color == CMODE) {	/* Reverse video.	*/
			ttputc(ESC);
			ttputc('p');
		}
		tthue = color;
	}
}

/*
 * No-op.
 */
ttresize()
{
}
