/*
 * Name:	MicroEMACS
 *		Assorted commands.
 * Version:	29
 * Last edit:	10-Feb-86
 * By:		rex::conroy
 *		decvax!decwrl!dec-rhea!dec-rex!conroy
 *
 * The file contains the command
 * processors for a large assortment of unrelated
 * commands. The only thing they have in common is
 * that they are all command processors.
 */
#include	"def.h"

/*
 * Display a bunch of useful information about
 * the current location of dot. The character under the
 * cursor (in octal), the current line, row, and column, and
 * approximate position of the cursor in the file (as a percentage)
 * is displayed. The column position assumes an infinite position
 * display; it does not truncate just because the screen does.
 * This is normally bound to "C-X =".
 */
showcpos(f, n, k)
{
	register LINE	*clp;
	register int	cbo;
	register int	nchar;
	register int	cchar;
	register int	nline;
	register int	cline;
	register int	cbyte;
	register int	ratio;
	register int	row;
	register int	col;
	register int	i;
	register int	c;

	clp = lforw(curbp->b_linep);		/* Collect the data.	*/
	cbo = 0;
	nchar = 0;
	nline = 1;				/* Origin 1.		*/
	for (;;) {
		if (clp == curwp->w_dotp) {
			cline = nline;
			if (cbo == curwp->w_doto) {
				cchar = nchar;
				if (cbo == llength(clp))
					cbyte = '\n';
				else
					cbyte = lgetc(clp, cbo);
			}
		}
		if (cbo == llength(clp)) {
			if (clp == curbp->b_linep)
				break;
			clp = lforw(clp);
			cbo = 0;
			++nline;		/* Count a line.	*/
		} else
			++cbo;
		++nchar;			/* Count a character.	*/
	}
	row = curwp->w_toprow;			/* Determine row.	*/
	clp = curwp->w_linep;
	while (clp!=curbp->b_linep && clp!=curwp->w_dotp) {
		++row;
		clp = lforw(clp);
	}
	++row;					/* Convert to origin 1.	*/
	col = 0;				/* Determine column.	*/
	for (i=0; i<curwp->w_doto; ++i) {
		c = lgetc(curwp->w_dotp, i);
		if (c == '\t')
			col |= 0x07;
		else if (ISCTRL(c) != FALSE)
			++col;
		++col;
	}
	++col;					/* Convert to origin 1.	*/
	ratio = 0;				/* Ratio before dot.	*/
	if (nchar != 0) {
		ratio = (100L*cchar) / nchar;
		if (ratio==0 && cchar!=0)	/* Allow 0% only at the	*/
			ratio = 1;		/* start of the file.	*/
	}
	eprintf("[CH:0%o Line:%d Row:%d Col:%d %d%% of %d]",
		cbyte, cline, row, col, ratio, nchar);
	return (TRUE);
}

/*
 * Twiddle the two characters on either side of
 * dot. If dot is at the end of the line twiddle the
 * two characters before it. Return with an error if dot
 * is at the beginning of line; it seems to be a bit
 * pointless to make this work. This fixes up a very
 * common typo with a single stroke. Normally bound
 * to "C-T". This always works within a line, so
 * "WFEDIT" is good enough.
 */
twiddle(f, n, k)
{
	register LINE	*dotp;
	register int	doto;
	register int	cl;
	register int	cr;

	dotp = curwp->w_dotp;
	doto = curwp->w_doto;
	if (doto==llength(dotp) && --doto<0)
		return (FALSE);
	cr = lgetc(dotp, doto);
	if (--doto < 0)
		return (FALSE);
	cl = lgetc(dotp, doto);
	lputc(dotp, doto+0, cr);
	lputc(dotp, doto+1, cl);
	lchange(WFEDIT);
	return (TRUE);
}

/*
 * Quote the next character, and
 * insert it into the buffer. All the characters
 * are taken literally, with the exception of the newline,
 * which always has its line splitting meaning. The character
 * is always read, even if it is inserted 0 times, for
 * regularity.
 */
quote(f, n, k)
{
	register int	s;
	register int	c;

	if (kbdmop != NULL)
		c = *kbdmop++;
	else {
		c = ttgetc();
		if (kbdmip != NULL) {
			if (kbdmip > &kbdm[NKBDM-4]) {
				ctrlg(FALSE, 0, KRANDOM);
				return (ABORT);
			}
			*kbdmip++ = c;
		}
	}
	if (n < 0)
		return (FALSE);
	if (n == 0)
		return (TRUE);
	if (c == '\n') {
		do {
			s = lnewline();
		} while (s==TRUE && --n);
		return (s);
	}
	return (linsert(n, c));
}

/*
 * Ordinary text characters are bound to this function,
 * which inserts them into the buffer. Characters marked as control
 * characters (using the CTRL flag) may be remapped to their ASCII
 * equivalent. This makes TAB (C-I) work right, and also makes the
 * world look reasonable if a control character is bound to this
 * this routine by hand. Any META or CTLX flags on the character
 * are discarded. This is the only routine that actually looks
 * the the "k" argument.
 */
selfinsert(f, n, k)
{
	register int	c;

	if (n < 0)
		return (FALSE);
	if (n == 0)
		return (TRUE);
	c = k & KCHAR;
	if ((k&KCTRL)!=0 && c>='@' && c<='_')	/* ASCII-ify.		*/
		c -= '@';
	return (linsert(n, c));
}

/*
 * Open up some blank space. The basic plan
 * is to insert a bunch of newlines, and then back
 * up over them. Everything is done by the subcommand
 * procerssors. They even handle the looping. Normally
 * this is bound to "C-O".
 */
openline(f, n, k)
{
	register int	i;
	register int	s;

	if (n < 0)
		return (FALSE);
	if (n == 0)
		return (TRUE);
	i = n;					/* Insert newlines.	*/
	do {
		s = lnewline();
	} while (s==TRUE && --i);
	if (s == TRUE)				/* Then back up overtop	*/
		s = backchar(f, n, KRANDOM);	/* of them all.		*/
	return (s);
}

/*
 * Insert a newline.
 * If you are at the end of the line and the
 * next line is a blank line, just move into the
 * blank line. This makes "C-O" and "C-X C-O" work
 * nicely, and reduces the ammount of screen
 * update that has to be done. This would not be
 * as critical if screen update were a lot
 * more efficient.
 */
newline(f, n, k)
{
	register LINE	*lp;
	register int	s;

	if (n < 0)
		return (FALSE);
	while (n--) {
		lp = curwp->w_dotp;
		if (llength(lp) == curwp->w_doto
		&& lp != curbp->b_linep
		&& llength(lforw(lp)) == 0) {
			if ((s=forwchar(FALSE, 1, KRANDOM)) != TRUE)
				return (s);
		} else if ((s=lnewline()) != TRUE)
			return (s);
	}
	return (TRUE);
}

/*
 * Delete blank lines around dot.
 * What this command does depends if dot is
 * sitting on a blank line. If dot is sitting on a
 * blank line, this command deletes all the blank lines
 * above and below the current line. If it is sitting
 * on a non blank line then it deletes all of the
 * blank lines after the line. Normally this command
 * is bound to "C-X C-O". Any argument is ignored.
 */
deblank(f, n, k)
{
	register LINE	*lp1;
	register LINE	*lp2;
	register int	nld;

	lp1 = curwp->w_dotp;
	while (llength(lp1)==0 && (lp2=lback(lp1))!=curbp->b_linep)
		lp1 = lp2;
	lp2 = lp1;
	nld = 0;
	while ((lp2=lforw(lp2))!=curbp->b_linep && llength(lp2)==0)
		++nld;
	if (nld == 0)
		return (TRUE);
	curwp->w_dotp = lforw(lp1);
	curwp->w_doto = 0;
	return (ldelete(nld, FALSE));
}

/*
 * Insert a newline, then enough
 * tabs and spaces to duplicate the indentation
 * of the previous line. Assumes tabs are every eight
 * characters. Quite simple. Figure out the indentation
 * of the current line. Insert a newline by calling
 * the standard routine. Insert the indentation by
 * inserting the right number of tabs and spaces.
 * Return TRUE if all ok. Return FALSE if one
 * of the subcomands failed. Normally bound
 * to "C-J".
 */
indent(f, n, k)
{
	register int	nicol;
	register int	c;
	register int	i;

	if (n < 0)
		return (FALSE);
	while (n--) {
		nicol = 0;
		for (i=0; i<llength(curwp->w_dotp); ++i) {
			c = lgetc(curwp->w_dotp, i);
			if (c!=' ' && c!='\t')
				break;
			if (c == '\t')
				nicol |= 0x07;
			++nicol;
		}
		if (lnewline() == FALSE
		|| ((i=nicol/8)!=0 && linsert(i, '\t')==FALSE)
		|| ((i=nicol%8)!=0 && linsert(i,  ' ')==FALSE))
			return (FALSE);
	}
	return (TRUE);
}

/*
 * Delete forward. This is real
 * easy, because the basic delete routine does
 * all of the work. Watches for negative arguments,
 * and does the right thing. If any argument is
 * present, it kills rather than deletes, to prevent
 * loss of text if typed with a big argument.
 * Normally bound to "C-D".
 */
forwdel(f, n, k)
{
	if (n < 0)
		return (backdel(f, -n, KRANDOM));
	if (f != FALSE) {			/* Really a kill.	*/
		if ((lastflag&CFKILL) == 0)
			kdelete();
		thisflag |= CFKILL;
	}
	return (ldelete(n, f));
}

/*
 * Delete backwards. This is quite easy too,
 * because it's all done with other functions. Just
 * move the cursor back, and delete forwards.
 * Like delete forward, this actually does a kill
 * if presented with an argument.
 */
backdel(f, n, k)
{
	register int	s;

	if (n < 0)
		return (forwdel(f, -n, KRANDOM));
	if (f != FALSE) {			/* Really a kill.	*/
		if ((lastflag&CFKILL) == 0)
			kdelete();
		thisflag |= CFKILL;
	}
	if ((s=backchar(f, n, KRANDOM)) == TRUE)
		s = ldelete(n, f);
	return (s);
}

/*
 * Kill line. If called without an argument,
 * it kills from dot to the end of the line, unless it
 * is at the end of the line, when it kills the newline.
 * If called with an argument of 0, it kills from the
 * start of the line to dot. If called with a positive
 * argument, it kills from dot forward over that number
 * of newlines. If called with a negative argument it
 * kills any text before dot on the current line,
 * then it kills back abs(arg) lines.
 */
killline(f, n, k)
{
	register int	chunk;
	register LINE	*nextp;

	if ((lastflag&CFKILL) == 0)		/* Clear kill buffer if	*/
		kdelete();			/* last wasn't a kill.	*/
	thisflag |= CFKILL;
	if (f == FALSE) {
		chunk = llength(curwp->w_dotp)-curwp->w_doto;
		if (chunk == 0)
			chunk = 1;
	} else if (n > 0) {
		chunk = llength(curwp->w_dotp)-curwp->w_doto+1;
		nextp = lforw(curwp->w_dotp);
		while (--n) {
			if (nextp == curbp->b_linep)
				return (FALSE);
			chunk += llength(nextp)+1;
			nextp = lforw(nextp);
		}
	} else {				/* n <= 0		*/
		chunk = curwp->w_doto;
		curwp->w_doto = 0;
		while (n++) {
			if (lback(curwp->w_dotp) == curbp->b_linep)
				break;
			curwp->w_dotp = lback(curwp->w_dotp);
			curwp->w_flag |= WFMOVE;
			chunk += llength(curwp->w_dotp)+1;
		}
	}
	return (ldelete(chunk, TRUE));
}

/*
 * Yank text back from the kill buffer. This
 * is really easy. All of the work is done by the
 * standard insert routines. All you do is run the loop,
 * and check for errors. The blank
 * lines are inserted with a call to "newline"
 * instead of a call to "lnewline" so that the magic
 * stuff that happens when you type a carriage
 * return also happens when a carriage return is
 * yanked back from the kill buffer.
 * An attempt has been made to fix the cosmetic bug
 * associated with a yank when dot is on the top line of
 * the window (nothing moves, because all of the new
 * text landed off screen).
 */
yank(f, n, k)
{
	register int	c;
	register int	i;
	register LINE	*lp;
	register int	nline;

	if (n < 0)
		return (FALSE);
	nline = 0;				/* Newline counting.	*/
	while (n--) {
		i = 0;
		while ((c=kremove(i)) >= 0) {
			if (c == '\n') {
				if (newline(FALSE, 1, KRANDOM) == FALSE)
					return (FALSE);
				++nline;
			} else {
				if (linsert(1, c) == FALSE)
					return (FALSE);
			}
			++i;
		}
	}
	lp = curwp->w_linep;			/* Cosmetic adjustment	*/
	if (curwp->w_dotp == lp) {		/* if offscreen insert.	*/
		while (nline-- && lback(lp)!=curbp->b_linep)
			lp = lback(lp);
		curwp->w_linep = lp;		/* Adjust framing.	*/
		curwp->w_flag |= WFHARD;
	}
	return (TRUE);
}
