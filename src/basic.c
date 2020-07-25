/*
 * Name:	MicroEMACS
 *		Basic cursor motion commands.
 * Version:	29
 * Last edit:	05-Feb-86
 * By:		rex::conroy
 *		decvax!decwrl!dec-rhea!dec-rex!conroy
 *
 * The routines in this file are the basic
 * command functions for moving the cursor around on
 * the screen, setting mark, and swapping dot with
 * mark. Only moves between lines, which might make the
 * current buffer framing bad, are hard.
 */
#include	"def.h"

/*
 * Go to beginning of line.
 */
gotobol(f, n, k)
{
	curwp->w_doto  = 0;
	return (TRUE);
}

/*
 * Move cursor backwards. Do the
 * right thing if the count is less than
 * 0. Error if you try to move back from
 * the beginning of the buffer.
 */
backchar(f, n, k)
register int	n;
{
	register LINE	*lp;

	if (n < 0)
		return (forwchar(f, -n, KRANDOM));
	while (n--) {
		if (curwp->w_doto == 0) {
			if ((lp=lback(curwp->w_dotp)) == curbp->b_linep)
				return (FALSE);
			curwp->w_dotp  = lp;
			curwp->w_doto  = llength(lp);
			curwp->w_flag |= WFMOVE;
		} else
			curwp->w_doto--;
	}
	return (TRUE);
}

/*
 * Go to end of line.
 */
gotoeol(f, n, k)
{
	curwp->w_doto  = llength(curwp->w_dotp);
	return (TRUE);
}

/*
 * Move cursor forwards. Do the
 * right thing if the count is less than
 * 0. Error if you try to move forward
 * from the end of the buffer.
 */
forwchar(f, n, k)
register int	n;
{
	if (n < 0)
		return (backchar(f, -n, KRANDOM));
	while (n--) {
		if (curwp->w_doto == llength(curwp->w_dotp)) {
			if (curwp->w_dotp == curbp->b_linep)
				return (FALSE);
			curwp->w_dotp  = lforw(curwp->w_dotp);
			curwp->w_doto  = 0;
			curwp->w_flag |= WFMOVE;
		} else
			curwp->w_doto++;
	}
	return (TRUE);
}

/*
 * Go to the beginning of the
 * buffer. Setting WFHARD is conservative,
 * but almost always the case.
 */
gotobob(f, n, k)
{
	curwp->w_dotp  = lforw(curbp->b_linep);
	curwp->w_doto  = 0;
	curwp->w_flag |= WFHARD;
	return (TRUE);
}

/*
 * Go to the end of the buffer.
 * Setting WFHARD is conservative, but
 * almost always the case.
 */
gotoeob(f, n, k)
{
	curwp->w_dotp  = curbp->b_linep;
	curwp->w_doto  = 0;
	curwp->w_flag |= WFHARD;
	return (TRUE);
}

/*
 * Move forward by full lines.
 * If the number of lines to move is less
 * than zero, call the backward line function to
 * actually do it. The last command controls how
 * the goal column is set.
 */
forwline(f, n, k)
{
	register LINE	*dlp;

	if (n < 0)
		return (backline(f, -n, KRANDOM));
	if ((lastflag&CFCPCN) == 0)		/* Fix goal.		*/
		setgoal();
	thisflag |= CFCPCN;
	dlp = curwp->w_dotp;
	while (n-- && dlp!=curbp->b_linep)
		dlp = lforw(dlp);
	curwp->w_dotp  = dlp;
	curwp->w_doto  = getgoal(dlp);
	curwp->w_flag |= WFMOVE;
	return (TRUE);
}

/*
 * This function is like "forwline", but
 * goes backwards. The scheme is exactly the same.
 * Check for arguments that are less than zero and
 * call your alternate. Figure out the new line and
 * call "movedot" to perform the motion.
 */
backline(f, n, k)
{
	register LINE	*dlp;

	if (n < 0)
		return (forwline(f, -n, KRANDOM));
	if ((lastflag&CFCPCN) == 0)		/* Fix goal.		*/
		setgoal();
	thisflag |= CFCPCN;
	dlp = curwp->w_dotp;
	while (n-- && lback(dlp)!=curbp->b_linep)
		dlp = lback(dlp);
	curwp->w_dotp  = dlp;
	curwp->w_doto  = getgoal(dlp);
	curwp->w_flag |= WFMOVE;
	return (TRUE);
}

/*
 * Set the current goal column,
 * which is saved in the external variable "curgoal",
 * to the current cursor column. The column is never off
 * the edge of the screen; it's more like display then
 * show position.
 */
setgoal()
{
	register int	c;
	register int	i;

	curgoal = 0;				/* Get the position.	*/
	for (i=0; i<curwp->w_doto; ++i) {
		c = lgetc(curwp->w_dotp, i);
		if (c == '\t')
			curgoal |= 0x07;
		else if (ISCTRL(c) != FALSE)
			++curgoal;
		++curgoal;
	}
	if (curgoal >= ncol)			/* Chop to tty width.	*/
		curgoal = ncol-1;
}

/*
 * This routine looks at a line (pointed
 * to by the LINE pointer "dlp") and the current
 * vertical motion goal column (set by the "setgoal"
 * routine above) and returns the best offset to use
 * when a vertical motion is made into the line.
 */
getgoal(dlp)
register LINE	*dlp;
{
	register int	c;
	register int	col;
	register int	newcol;
	register int	dbo;

	col = 0;
	dbo = 0;
	while (dbo != llength(dlp)) {
		c = lgetc(dlp, dbo);
		newcol = col;
		if (c == '\t')
			newcol |= 0x07;
		else if (ISCTRL(c) != FALSE)
			++newcol;
		++newcol;
		if (newcol > curgoal)
			break;
		col = newcol;
		++dbo;
	}
	return (dbo);
}

/*
 * Scroll forward by a specified number
 * of lines, or by a full page if no argument.
 * The "2" is the window overlap (this is the default
 * value from ITS EMACS). Because the top line in
 * the window is zapped, we have to do a hard
 * update and get it back.
 */
forwpage(f, n, k)
register int	n;
{
	register LINE	*lp;

	if (f == FALSE) {
		n = curwp->w_ntrows - 2;	/* Default scroll.	*/
		if (n <= 0)			/* Forget the overlap	*/
			n = 1;			/* if tiny window.	*/
	} else if (n < 0)
		return (backpage(f, -n, KRANDOM));
#if	CVMVAS
	else					/* Convert from pages	*/
		n *= curwp->w_ntrows;		/* to lines.		*/
#endif
	lp = curwp->w_linep;
	while (n-- && lp!=curbp->b_linep)
		lp = lforw(lp);
	curwp->w_linep = lp;
	curwp->w_dotp  = lp;
	curwp->w_doto  = 0;
	curwp->w_flag |= WFHARD;
	return (TRUE);
}

/*
 * This command is like "forwpage",
 * but it goes backwards. The "2", like above,
 * is the overlap between the two windows. The
 * value is from the ITS EMACS manual. The
 * hard update is done because the top line in
 * the window is zapped.
 */
backpage(f, n, k)
register int	n;
{
	register LINE	*lp;

	if (f == FALSE) {
		n = curwp->w_ntrows - 2;	/* Default scroll.	*/
		if (n <= 0)			/* Don't blow up if the	*/
			n = 1;			/* window is tiny.	*/
	} else if (n < 0)
		return (forwpage(f, -n, KRANDOM));
#if	CVMVAS
	else					/* Convert from pages	*/
		n *= curwp->w_ntrows;		/* to lines.		*/
#endif
	lp = curwp->w_linep;
	while (n-- && lback(lp)!=curbp->b_linep)
		lp = lback(lp);
	curwp->w_linep = lp;
	curwp->w_dotp  = lp;
	curwp->w_doto  = 0;
	curwp->w_flag |= WFHARD;
	return (TRUE);
}

/*
 * Set the mark in the current window
 * to the value of dot. A message is written to
 * the echo line unless we are running in a keyboard
 * macro, when it would be silly.
 */
setmark(f, n, k)
{
	curwp->w_markp = curwp->w_dotp;
	curwp->w_marko = curwp->w_doto;
	if (kbdmop == NULL)
		eprintf("[Mark set]");
	return (TRUE);
}

/*
 * Swap the values of "dot" and "mark" in
 * the current window. This is pretty easy, because
 * all of the hard work gets done by the standard routine
 * that moves the mark about. The only possible
 * error is "no mark".
 */
swapmark(f, n, k)
{
	register LINE	*odotp;
	register int	odoto;

	if (curwp->w_markp == NULL) {
		eprintf("No mark in this window");
		return (FALSE);
	}
	odotp = curwp->w_dotp;
	odoto = curwp->w_doto;
	curwp->w_dotp  = curwp->w_markp;
	curwp->w_doto  = curwp->w_marko;
	curwp->w_markp = odotp;
	curwp->w_marko = odoto;
	curwp->w_flag |= WFMOVE;
	return (TRUE);
}

/*
 * Go to a specific line, mostly for
 * looking up errors in C programs, which give the
 * error a line number. If an argument is present, then
 * it is the line number, else prompt for a line number
 * to use.
 */
gotoline(f, n, k)
register int	n;
{
	register LINE	*clp;
	register int	s;
	char		buf[32];

	if (f == FALSE) {
		if ((s=ereply("Goto line: ", buf, sizeof(buf))) != TRUE)
			return (s);
		n = atoi(buf);
	}
	if (n <= 0) {
		eprintf("Bad line");
		return (FALSE);
	}
	clp = lforw(curbp->b_linep);		/* "clp" is first line	*/
	while (n != 1) {
		if (clp == curbp->b_linep) {
			eprintf("Line number too large");
			return (FALSE);
		}
		clp = lforw(clp);
		--n;
	}
	curwp->w_dotp = clp;
	curwp->w_doto = 0;
	curwp->w_flag |= WFMOVE;
	return (TRUE);
}
