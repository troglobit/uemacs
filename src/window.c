/*
 * Name:	MicroEMACS
 *		Window handling.
 * Version:	29
 * Last edit:	10-Feb-86
 * By:		rex::conroy
 *		decvax!decwrl!dec-rhea!dec-rex!conroy
 */
#include	"def.h"

/*
 * Reposition dot in the current
 * window to line "n". If the argument is
 * positive, it is that line. If it is negative it
 * is that line from the bottom. If it is 0 the window
 * is centered (this is what the standard redisplay code
 * does). With no argument it defaults to 1.
 * Because of the default, it works like in
 * Gosling.
 */
reposition(f, n, k)
{
	curwp->w_force = n;
	curwp->w_flag |= WFFORCE;
	return (TRUE);
}

/*
 * Refresh the display. A call is made to the
 * "ttresize" entry in the terminal handler, which tries
 * to reset "nrow" and "ncol". They will, however, never
 * be set outside of the NROW or NCOL range. If the display
 * changed size, arrange that everything is redone, then
 * call "update" to fix the display. We do this so the
 * new size can be displayed. In the normal case the
 * call to "update" in "main.c" refreshes the screen,
 * and all of the windows need not be recomputed.
 * Note that when you get to the "display unusable"
 * message, the screen will be messed up. If you make
 * the window bigger again, and send another command,
 * everything will get fixed!
 */
refresh(f, n, k)
{
	register WINDOW	*wp;
	register int	oldnrow;
	register int	oldncol;

	oldnrow = nrow;
	oldncol = ncol;
	ttresize();
	if (nrow!=oldnrow || ncol!=oldncol) {
		wp = wheadp;			/* Find last.		*/
		while (wp->w_wndp != NULL)
			wp = wp->w_wndp;
		if (nrow < wp->w_toprow+3) {	/* Check if too small.	*/
			eprintf("Display unusable");
			return (FALSE);
		}		
		wp->w_ntrows = nrow-wp->w_toprow-2;
		wp = wheadp;			/* Redraw all.		*/
		while (wp != NULL) {
			wp->w_flag |= WFMODE|WFHARD;
			wp = wp->w_wndp;
		}
		sgarbf = TRUE;
		update();
		eprintf("[New size %d by %d]", nrow, ncol);
	} else
		sgarbf = TRUE;
	return (TRUE);
}

/*
 * The command make the next
 * window (next => down the screen)
 * the current window. There are no real
 * errors, although the command does
 * nothing if there is only 1 window on
 * the screen.
 */
nextwind(f, n, k)
{
	register WINDOW	*wp;

	if ((wp=curwp->w_wndp) == NULL)
		wp = wheadp;
	curwp = wp;
	curbp = wp->w_bufp;
	return (TRUE);
}

/*
 * This command makes the previous
 * window (previous => up the screen) the
 * current window. There arn't any errors,
 * although the command does not do a lot
 * if there is 1 window.
 */
prevwind(f, n, k)
{
	register WINDOW	*wp1;
	register WINDOW	*wp2;

	wp1 = wheadp;
	wp2 = curwp;
	if (wp1 == wp2)
		wp2 = NULL;
	while (wp1->w_wndp != wp2)
		wp1 = wp1->w_wndp;
	curwp = wp1;
	curbp = wp1->w_bufp;
	return (TRUE);
}

/*
 * This command moves the current
 * window down by "arg" lines. Recompute
 * the top line in the window. The move up and
 * move down code is almost completely the same;
 * most of the work has to do with reframing the
 * window, and picking a new dot. We share the
 * code by having "move down" just be an interface
 * to "move up".
 */
mvdnwind(f, n, k)
register int	n;
{
	return (mvupwind(f, -n, KRANDOM));
}

/*
 * Move the current window up by "arg"
 * lines. Recompute the new top line of the window.
 * Look to see if "." is still on the screen. If it is,
 * you win. If it isn't, then move "." to center it
 * in the new framing of the window (this command does
 * not really move "."; it moves the frame).
 */
mvupwind(f, n, k)
register int	n;
{
	register LINE	*lp;
	register int	i;

	lp = curwp->w_linep;
	if (n < 0) {
		while (n++ && lp!=curbp->b_linep)
			lp = lforw(lp);
	} else {
		while (n-- && lback(lp)!=curbp->b_linep)
			lp = lback(lp);
	}
	curwp->w_linep = lp;
	curwp->w_flag |= WFHARD;		/* Mode line is OK.	*/
	for (i=0; i<curwp->w_ntrows; ++i) {
		if (lp == curwp->w_dotp)
			return (TRUE);
		if (lp == curbp->b_linep)
			break;
		lp = lforw(lp);
	}
	lp = curwp->w_linep;
	i  = curwp->w_ntrows/2;
	while (i-- && lp!=curbp->b_linep)
		lp = lforw(lp);
	curwp->w_dotp  = lp;
	curwp->w_doto  = 0;
	return (TRUE);
}

/*
 * This command makes the current
 * window the only window on the screen.
 * Try to set the framing
 * so that "." does not have to move on
 * the display. Some care has to be taken
 * to keep the values of dot and mark
 * in the buffer structures right if the
 * distruction of a window makes a buffer
 * become undisplayed.
 */
onlywind(f, n, k)
{
	register WINDOW	*wp;
	register LINE	*lp;
	register int	i;

	while (wheadp != curwp) {
		wp = wheadp;
		wheadp = wp->w_wndp;
		if (--wp->w_bufp->b_nwnd == 0) {
			wp->w_bufp->b_dotp  = wp->w_dotp;
			wp->w_bufp->b_doto  = wp->w_doto;
			wp->w_bufp->b_markp = wp->w_markp;
			wp->w_bufp->b_marko = wp->w_marko;
		}
		free((char *) wp);
	}
	while (curwp->w_wndp != NULL) {
		wp = curwp->w_wndp;
		curwp->w_wndp = wp->w_wndp;
		if (--wp->w_bufp->b_nwnd == 0) {
			wp->w_bufp->b_dotp  = wp->w_dotp;
			wp->w_bufp->b_doto  = wp->w_doto;
			wp->w_bufp->b_markp = wp->w_markp;
			wp->w_bufp->b_marko = wp->w_marko;
		}
		free((char *) wp);
	}
	lp = curwp->w_linep;
	i  = curwp->w_toprow;
	while (i!=0 && lback(lp)!=curbp->b_linep) {
		--i;
		lp = lback(lp);
	}
	curwp->w_toprow = 0;
	curwp->w_ntrows = nrow-2;		/* 2 = mode, echo.	*/
	curwp->w_linep  = lp;
	curwp->w_flag  |= WFMODE|WFHARD;
	return (TRUE);
}

/*
 * Split the current window. A window
 * smaller than 3 lines cannot be split.
 * The only other error that is possible is
 * a "malloc" failure allocating the structure
 * for the new window.
 */
splitwind(f, n, k)
{
	register WINDOW	*wp;
	register LINE	*lp;
	register int	ntru;
	register int	ntrl;
	register int	ntrd;
	register WINDOW	*wp1;
	register WINDOW	*wp2;

	if (curwp->w_ntrows < 3) {
		eprintf("Cannot split a %d line window", curwp->w_ntrows);
		return (FALSE);
	}
	if ((wp = (WINDOW *) malloc(sizeof(WINDOW))) == NULL) {
		eprintf("Cannot allocate WINDOW block");
		return (FALSE);
	}
	++curbp->b_nwnd;			/* Displayed twice.	*/
	wp->w_bufp  = curbp;
	wp->w_dotp  = curwp->w_dotp;
	wp->w_doto  = curwp->w_doto;
	wp->w_markp = curwp->w_markp;
	wp->w_marko = curwp->w_marko;
	wp->w_flag  = 0;
	wp->w_force = 0;
	ntru = (curwp->w_ntrows-1) / 2;		/* Upper size		*/
	ntrl = (curwp->w_ntrows-1) - ntru;	/* Lower size		*/
	lp = curwp->w_linep;
	ntrd = 0;
	while (lp != curwp->w_dotp) {
		++ntrd;
		lp = lforw(lp);
	}
	lp = curwp->w_linep;
	if (ntrd <= ntru) {			/* Old is upper window.	*/
		if (ntrd == ntru)		/* Hit mode line.	*/
			lp = lforw(lp);
		curwp->w_ntrows = ntru;
		wp->w_wndp = curwp->w_wndp;
		curwp->w_wndp = wp;
		wp->w_toprow = curwp->w_toprow+ntru+1;
		wp->w_ntrows = ntrl;
	} else {				/* Old is lower window	*/
		wp1 = NULL;
		wp2 = wheadp;
		while (wp2 != curwp) {
			wp1 = wp2;
			wp2 = wp2->w_wndp;
		}
		if (wp1 == NULL)
			wheadp = wp;
		else
			wp1->w_wndp = wp;
		wp->w_wndp   = curwp;
		wp->w_toprow = curwp->w_toprow;
		wp->w_ntrows = ntru;
		++ntru;				/* Mode line.		*/
		curwp->w_toprow += ntru;
		curwp->w_ntrows  = ntrl;
		while (ntru--)
			lp = lforw(lp);
	}
	curwp->w_linep = lp;			/* Adjust the top lines	*/
	wp->w_linep = lp;			/* if necessary.	*/
	curwp->w_flag |= WFMODE|WFHARD;
	wp->w_flag |= WFMODE|WFHARD;
	return (TRUE);
}

/*
 * Enlarge the current window.
 * Find the window that loses space. Make
 * sure it is big enough. If so, hack the window
 * descriptions, and ask redisplay to do all the
 * hard work. You don't just set "force reframe"
 * because dot would move.
 */
enlargewind(f, n, k)
{
	register WINDOW	*adjwp;
	register LINE	*lp;
	register int	i;

	if (n < 0)
		return (shrinkwind(f, -n, KRANDOM));
	if (wheadp->w_wndp == NULL) {
		eprintf("Only one window");
		return (FALSE);
	}
	if ((adjwp=curwp->w_wndp) == NULL) {
		adjwp = wheadp;
		while (adjwp->w_wndp != curwp)
			adjwp = adjwp->w_wndp;
	}
	if (adjwp->w_ntrows <= n) {
		eprintf("Impossible change");
		return (FALSE);
	}
	if (curwp->w_wndp == adjwp) {		/* Shrink below.	*/
		lp = adjwp->w_linep;
		for (i=0; i<n && lp!=adjwp->w_bufp->b_linep; ++i)
			lp = lforw(lp);
		adjwp->w_linep  = lp;
		adjwp->w_toprow += n;
	} else {				/* Shrink above.	*/
		lp = curwp->w_linep;
		for (i=0; i<n && lback(lp)!=curbp->b_linep; ++i)
			lp = lback(lp);
		curwp->w_linep  = lp;
		curwp->w_toprow -= n;
	}
	curwp->w_ntrows += n;
	adjwp->w_ntrows -= n;
	curwp->w_flag |= WFMODE|WFHARD;
	adjwp->w_flag |= WFMODE|WFHARD;
	return (TRUE);
}

/*
 * Shrink the current window.
 * Find the window that gains space. Hack at
 * the window descriptions. Ask the redisplay to
 * do all the hard work.
 */
shrinkwind(f, n, k)
{
	register WINDOW	*adjwp;
	register LINE	*lp;
	register int	i;

	if (n < 0)
		return (enlargewind(f, -n, KRANDOM));
	if (wheadp->w_wndp == NULL) {
		eprintf("Only one window");
		return (FALSE);
	}
	if ((adjwp=curwp->w_wndp) == NULL) {
		adjwp = wheadp;
		while (adjwp->w_wndp != curwp)
			adjwp = adjwp->w_wndp;
	}
	if (curwp->w_ntrows <= n) {
		eprintf("Impossible change");
		return (FALSE);
	}
	if (curwp->w_wndp == adjwp) {		/* Grow below.		*/
		lp = adjwp->w_linep;
		for (i=0; i<n && lback(lp)!=adjwp->w_bufp->b_linep; ++i)
			lp = lback(lp);
		adjwp->w_linep  = lp;
		adjwp->w_toprow -= n;
	} else {				/* Grow above.		*/
		lp = curwp->w_linep;
		for (i=0; i<n && lp!=curbp->b_linep; ++i)
			lp = lforw(lp);
		curwp->w_linep  = lp;
		curwp->w_toprow += n;
	}
	curwp->w_ntrows -= n;
	adjwp->w_ntrows += n;
	curwp->w_flag |= WFMODE|WFHARD;
	adjwp->w_flag |= WFMODE|WFHARD;
	return (TRUE);
}

/*
 * Pick a window for a pop-up.
 * Split the screen if there is only
 * one window. Pick the uppermost window that
 * isn't the current window. An LRU algorithm
 * might be better. Return a pointer, or
 * NULL on error.
 */
WINDOW	*
wpopup()
{
	register WINDOW	*wp;

	if (wheadp->w_wndp == NULL
	&& splitwind(FALSE, 0, KRANDOM) == FALSE)
		return (NULL);
	wp = wheadp;				/* Find window to use	*/
	while (wp!=NULL && wp==curwp)
		wp = wp->w_wndp;
	return (wp);
}
