/*
 * Name:	MicroEMACS
 *		Gosling style redisplay.
 * Version:	30
 * Last edit:	10-Feb-86
 * By:		rex::conroy
 *		decvax!decwrl!dec-rhea!dec-rex!conroy
 *
 * The functions in this file handle redisplay. The
 * redisplay system knows almost nothing about the editing
 * process; the editing functions do, however, set some
 * hints to eliminate a lot of the grinding. There is more
 * that can be done; the "vtputc" interface is a real
 * pig. Two conditional compilation flags; the GOSLING
 * flag enables dynamic programming redisplay, using the
 * algorithm published by Jim Gosling in SIGOA. The MEMMAP
 * changes things around for memory mapped video. With
 * both off, the terminal is a VT52.
 */
#include	"def.h"

/*
 * You can change these back to the types
 * implied by the name if you get tight for space. If you
 * make both of them "int" you get better code on the VAX.
 * They do nothing if this is not Gosling redisplay, except
 * for change the size of a structure that isn't used.
 * A bit of a cheat.
 */
#define	XCHAR	int
#define	XSHORT	int

/*
 * A video structure always holds
 * an array of characters whose length is equal to
 * the longest line possible. Only some of this is
 * used if "ncol" isn't the same as "NCOL".
 */
typedef	struct	{
	short	v_hash;			/* Hash code, for compares.	*/
	short	v_flag;			/* Flag word.			*/
	short	v_color;		/* Color of the line.		*/
	XSHORT	v_cost;			/* Cost of display.		*/
	char	v_text[NCOL];		/* The actual characters.	*/
}	VIDEO;

#define	VFCHG	0x0001			/* Changed.			*/
#define	VFHBAD	0x0002			/* Hash and cost are bad.	*/

/*
 * SCORE structures hold the optimal
 * trace trajectory, and the cost of redisplay, when
 * the dynamic programming redisplay code is used.
 * If no fancy redisplay, this isn't used. The trace index
 * fields can be "char", and the score a "short", but
 * this makes the code worse on the VAX.
 */
typedef	struct	{
	XCHAR	s_itrace;		/* "i" index for track back.	*/
	XCHAR	s_jtrace;		/* "j" index for trace back.	*/
	XSHORT	s_cost;			/* Display cost.		*/
}	SCORE;

int	sgarbf	= TRUE;			/* TRUE if screen is garbage.	*/
int	vtrow	= 0;			/* Virtual cursor row.		*/
int	vtcol	= 0;			/* Virtual cursor column.	*/
int	tthue	= CNONE;		/* Current color.		*/
int	ttrow	= HUGE;			/* Physical cursor row.		*/
int	ttcol	= HUGE;			/* Physical cursor column.	*/
int	tttop	= HUGE;			/* Top of scroll region.	*/
int	ttbot	= HUGE;			/* Bottom of scroll region.	*/

VIDEO	*vscreen[NROW-1];		/* Edge vector, virtual.	*/
VIDEO	*pscreen[NROW-1];		/* Edge vector, physical.	*/
VIDEO	video[2*(NROW-1)];		/* Actual screen data.		*/
VIDEO	blanks;				/* Blank line image.		*/

#if	GOSLING
/*
 * This matrix is written as an array because
 * we do funny things in the "setscores" routine, which
 * is very compute intensive, to make the subscripts go away.
 * It would be "SCORE	score[NROW][NROW]" in old speak.
 * Look at "setscores" to understand what is up.
 */
SCORE	score[NROW*NROW];
#endif

/*
 * Initialize the data structures used
 * by the display code. The edge vectors used
 * to access the screens are set up. The operating
 * system's terminal I/O channel is set up. Fill the
 * "blanks" array with ASCII blanks. The rest is done
 * at compile time. The original window is marked
 * as needing full update, and the physical screen
 * is marked as garbage, so all the right stuff happens
 * on the first call to redisplay.
 */
vtinit()
{
	register VIDEO	*vp;
	register int	i;

	ttopen();
	ttinit();
	vp = &video[0];
	for (i=0; i<NROW-1; ++i) {
		vscreen[i] = vp;
		++vp;
		pscreen[i] = vp;
		++vp;
	}
	blanks.v_color = CTEXT;
	for (i=0; i<NCOL; ++i)
		blanks.v_text[i] = ' ';
}

/*
 * Tidy up the virtual display system
 * in anticipation of a return back to the host
 * operating system. Right now all we do is position
 * the cursor to the last line, erase the line, and
 * close the terminal channel.
 */
vttidy()
{
	ttcolor(CTEXT);
	ttnowindow();				/* No scroll window.	*/
	ttmove(nrow-1, 0);			/* Echo line.		*/
	tteeol();
	tttidy();
	ttflush();
	ttclose();
}

/*
 * Move the virtual cursor to an origin
 * 0 spot on the virtual display screen. I could
 * store the column as a character pointer to the spot
 * on the line, which would make "vtputc" a little bit
 * more efficient. No checking for errors.
 */
vtmove(row, col)
{
	vtrow = row;
	vtcol = col;
}

/*
 * Write a character to the virtual display,
 * dealing with long lines and the display of unprintable
 * things like control characters. Also expand tabs every 8
 * columns. This code only puts printing characters into 
 * the virtual display image. Special care must be taken when
 * expanding tabs. On a screen whose width is not a multiple
 * of 8, it is possible for the virtual cursor to hit the
 * right margin before the next tab stop is reached. This
 * makes the tab code loop if you are not careful.
 * Three guesses how we found this.
 */
vtputc(c)
register int	c;
{
	register VIDEO	*vp;

	vp = vscreen[vtrow];
	if (vtcol >= ncol)
		vp->v_text[ncol-1] = '$';
	else if (c == '\t') {
		do {
			vtputc(' ');
		} while (vtcol<ncol && (vtcol&0x07)!=0);
	} else if (ISCTRL(c) != FALSE) {
		vtputc('^');
		vtputc(c ^ 0x40);
	} else
		vp->v_text[vtcol++] = c;		
}

/*
 * Erase from the end of the
 * software cursor to the end of the
 * line on which the software cursor is
 * located. The display routines will decide
 * if a hardware erase to end of line command
 * should be used to display this.
 */
vteeol()
{
	register VIDEO	*vp;

	vp = vscreen[vtrow];
	while (vtcol < ncol)
		vp->v_text[vtcol++] = ' ';
}

/*
 * Make sure that the display is
 * right. This is a three part process. First,
 * scan through all of the windows looking for dirty
 * ones. Check the framing, and refresh the screen.
 * Second, make sure that "currow" and "curcol" are
 * correct for the current window. Third, make the
 * virtual and physical screens the same.
 */
update()
{
	register LINE	*lp;
	register WINDOW	*wp;
	register VIDEO	*vp1;
	register VIDEO	*vp2;
	register int	i;
	register int	j;
	register int	c;
	register int	hflag;
	register int	currow;
	register int	curcol;
	register int	offs;
	register int	size;

	if (curmsgf!=FALSE || newmsgf!=FALSE) {
		wp = wheadp;
		while (wp != NULL) {
			wp->w_flag |= WFMODE;	/* Must do mode lines.	*/
			wp = wp->w_wndp;
		}
	}
	curmsgf = newmsgf;			/* Sync. up right now.	*/
	hflag = FALSE;				/* Not hard.		*/
	wp = wheadp;
	while (wp != NULL) {
		if (wp->w_flag != 0) {		/* Need update.		*/
			if ((wp->w_flag&WFFORCE) == 0) {
				lp = wp->w_linep;
				for (i=0; i<wp->w_ntrows; ++i) {
					if (lp == wp->w_dotp)
						goto out;
					if (lp == wp->w_bufp->b_linep)
						break;
					lp = lforw(lp);
				}
			}
			i = wp->w_force;	/* Reframe this one.	*/
			if (i > 0) {
				--i;
				if (i >= wp->w_ntrows)
					i = wp->w_ntrows-1;
			} else if (i < 0) {
				i += wp->w_ntrows;
				if (i < 0)
					i = 0;
			} else
				i = wp->w_ntrows/2;
			lp = wp->w_dotp;
			while (i!=0 && lback(lp)!=wp->w_bufp->b_linep) {
				--i;
				lp = lback(lp);
			}
			wp->w_linep = lp;
			wp->w_flag |= WFHARD;	/* Force full.		*/
		out:
			lp = wp->w_linep;	/* Try reduced update.	*/
			i  = wp->w_toprow;
			if ((wp->w_flag&~WFMODE) == WFEDIT) {
				while (lp != wp->w_dotp) {
					++i;
					lp = lforw(lp);
				}
				vscreen[i]->v_color = CTEXT;
				vscreen[i]->v_flag |= (VFCHG|VFHBAD);
				vtmove(i, 0);
				for (j=0; j<llength(lp); ++j)
					vtputc(lgetc(lp, j));
				vteeol();
			} else if ((wp->w_flag&(WFEDIT|WFHARD)) != 0) {
				hflag = TRUE;
				while (i < wp->w_toprow+wp->w_ntrows) {
					vscreen[i]->v_color = CTEXT;
					vscreen[i]->v_flag |= (VFCHG|VFHBAD);
					vtmove(i, 0);
					if (lp != wp->w_bufp->b_linep) {
						for (j=0; j<llength(lp); ++j)
							vtputc(lgetc(lp, j));
						lp = lforw(lp);
					}
					vteeol();
					++i;
				}
			}
			if ((wp->w_flag&WFMODE) != 0)
				modeline(wp);
			wp->w_flag  = 0;
			wp->w_force = 0;
		}		
		wp = wp->w_wndp;
	}
	lp = curwp->w_linep;			/* Cursor location.	*/
	currow = curwp->w_toprow;
	while (lp != curwp->w_dotp) {
		++currow;
		lp = lforw(lp);
	}
	curcol = 0;
	i = 0;
	while (i < curwp->w_doto) {
		c = lgetc(lp, i++);
		if (c == '\t')
			curcol |= 0x07;
		else if (ISCTRL(c) != FALSE)
			++curcol;
		++curcol;
	}
	if (curcol >= ncol)			/* Long line.		*/
		curcol = ncol-1;
	if (sgarbf != FALSE) {			/* Screen is garbage.	*/
		sgarbf = FALSE;			/* Erase-page clears	*/
		epresf = FALSE;			/* the message area.	*/
		tttop  = HUGE;			/* Forget where you set	*/
		ttbot  = HUGE;			/* scroll region.	*/
		tthue  = CNONE;			/* Color unknown.	*/
		ttmove(0, 0);
		tteeop();
		for (i=0; i<nrow-1; ++i) {
			uline(i, vscreen[i], &blanks);
			ucopy(vscreen[i], pscreen[i]);
		}
		ttmove(currow, curcol);
		ttflush();
		return;
	}
#if	GOSLING
	if (hflag != FALSE) {			/* Hard update?		*/
		for (i=0; i<nrow-1; ++i) {	/* Compute hash data.	*/
			hash(vscreen[i]);
			hash(pscreen[i]);
		}
		offs = 0;			/* Get top match.	*/
		while (offs != nrow-1) {
			vp1 = vscreen[offs];
			vp2 = pscreen[offs];
			if (vp1->v_color != vp2->v_color
			||  vp1->v_hash  != vp2->v_hash)
				break;
			uline(offs, vp1, vp2);
			ucopy(vp1, vp2);
			++offs;
		}
		if (offs == nrow-1) {		/* Might get it all.	*/
			ttmove(currow, curcol);
			ttflush();
			return;
		}
		size = nrow-1;			/* Get bottom match.	*/
		while (size != offs) {
			vp1 = vscreen[size-1];
			vp2 = pscreen[size-1];
			if (vp1->v_color != vp2->v_color
			||  vp1->v_hash  != vp2->v_hash)
				break;
			uline(size-1, vp1, vp2);
			ucopy(vp1, vp2);
			--size;
		}
		if ((size -= offs) == 0)	/* Get screen size.	*/
			abort();
		setscores(offs, size);		/* Do hard update.	*/
		traceback(offs, size, size, size);
		for (i=0; i<size; ++i)
			ucopy(vscreen[offs+i], pscreen[offs+i]);
		ttmove(currow, curcol);
		ttflush();
		return;			
	}
#endif
	for (i=0; i<nrow-1; ++i) {		/* Easy update.		*/
		vp1 = vscreen[i];
		vp2 = pscreen[i];
		if ((vp1->v_flag&VFCHG) != 0) {
			uline(i, vp1, vp2);
			ucopy(vp1, vp2);
		}
	}
	ttmove(currow, curcol);
	ttflush();
}

/*
 * Update a saved copy of a line,
 * kept in a VIDEO structure. The "vvp" is
 * the one in the "vscreen". The "pvp" is the one
 * in the "pscreen". This is called to make the
 * virtual and physical screens the same when
 * display has done an update.
 */
ucopy(vvp, pvp)
register VIDEO	*vvp;
register VIDEO	*pvp;
{
	register int	i;

	vvp->v_flag &= ~VFCHG;			/* Changes done.	*/
	pvp->v_flag  = vvp->v_flag;		/* Update model.	*/
	pvp->v_hash  = vvp->v_hash;
	pvp->v_cost  = vvp->v_cost;
	pvp->v_color = vvp->v_color;
	for (i=0; i<ncol; ++i)
		pvp->v_text[i] = vvp->v_text[i];
}

/*
 * Update a single line. This routine only
 * uses basic functionality (no insert and delete character,
 * but erase to end of line). The "vvp" points at the VIDEO
 * structure for the line on the virtual screen, and the "pvp"
 * is the same for the physical screen. Avoid erase to end of
 * line when updating CMODE color lines, because of the way that
 * reverse video works on most terminals.
 */
uline(row, vvp, pvp)
VIDEO	*vvp;
VIDEO	*pvp;
{
#if	MEMMAP
	putline(row+1, 1, &vvp->v_text[0]);
#else
	register char	*cp1;
	register char	*cp2;
	register char	*cp3;
	register char	*cp4;
	register char	*cp5;
	register int	nbflag;

	if (vvp->v_color != pvp->v_color) {	/* Wrong color, do a	*/
		ttmove(row, 0);			/* full redraw.		*/
		ttcolor(vvp->v_color);
		cp1 = &vvp->v_text[0];
		cp2 = &vvp->v_text[ncol];
		while (cp1 != cp2) {
			ttputc(*cp1++);
			++ttcol;
		}
		return;
	}
	cp1 = &vvp->v_text[0];			/* Compute left match.	*/
	cp2 = &pvp->v_text[0];
	while (cp1!=&vvp->v_text[ncol] && cp1[0]==cp2[0]) {
		++cp1;
		++cp2;
	}
	if (cp1 == &vvp->v_text[ncol])		/* All equal.		*/
		return;
	nbflag = FALSE;
	cp3 = &vvp->v_text[ncol];		/* Compute right match.	*/
	cp4 = &pvp->v_text[ncol];
	while (cp3[-1] == cp4[-1]) {
		--cp3;
		--cp4;
		if (cp3[0] != ' ')		/* Note non-blanks in	*/
			nbflag = TRUE;		/* the right match.	*/
	}
	cp5 = cp3;				/* Is erase good?	*/
	if (nbflag==FALSE && vvp->v_color==CTEXT) {
		while (cp5!=cp1 && cp5[-1]==' ')
			--cp5;
		/* Alcyon hack */
		if ((int)(cp3-cp5) <= tceeol)
			cp5 = cp3;
	}
	/* Alcyon hack */
	ttmove(row, (int)(cp1-&vvp->v_text[0]));
	ttcolor(vvp->v_color);
	while (cp1 != cp5) {
		ttputc(*cp1++);
		++ttcol;
	}
	if (cp5 != cp3)				/* Do erase.		*/
		tteeol();
#endif
}

/*
 * Redisplay the mode line for
 * the window pointed to by the "wp".
 * This is the only routine that has any idea
 * of how the modeline is formatted. You can
 * change the modeline format by hacking at
 * this routine. Called by "update" any time
 * there is a dirty window.
 */
modeline(wp)
register WINDOW	*wp;
{
	register char	*cp;
	register int	c;
	register int	n;
	register BUFFER	*bp;

	n = wp->w_toprow+wp->w_ntrows;		/* Location.		*/
	vscreen[n]->v_color = CMODE;		/* Mode line color.	*/
	vscreen[n]->v_flag |= (VFCHG|VFHBAD);	/* Recompute, display.	*/
	vtmove(n, 0);				/* Seek to right line.	*/
	bp = wp->w_bufp;
	if ((bp->b_flag&BFCHG) != 0)		/* "*" if changed.	*/
		vtputc('*');
	else
		vtputc(' ');
	n  = 1;
	cp = "MicroEMACS";			/* Buffer name.		*/
	while ((c = *cp++) != 0) {
		vtputc(c);
		++n;
	}
	if (bp->b_bname[0] != 0) {
		vtputc(' ');
		++n;
		cp = &bp->b_bname[0];
		while ((c = *cp++) != 0) {
			vtputc(c);
			++n;
		}
	}
	if (bp->b_fname[0] != 0) {		/* File name.		*/
		vtputc(' ');
		++n;
		cp = "File:";
		while ((c = *cp++) != 0) {
			vtputc(c);
			++n;
		}
		cp = &bp->b_fname[0];
		while ((c = *cp++) != 0) {
			vtputc(c);
			++n;
		}
	}
	if (curmsgf != FALSE			/* Message alert.	*/
	&& wp->w_wndp == NULL) {
		while (n < ncol-5-1) {
			vtputc(' ');
			++n;
		}
		cp = "[Msg]";			/* Sizeof("[Msg]") = 5.	*/
		while ((c = *cp++) != 0) {
			vtputc(c);
			++n;
		}
	}
	while (n < ncol) {			/* Pad out.		*/
		vtputc(' ');
		++n;
	}
}

#if	GOSLING
/*
 * Compute the hash code for
 * the line pointed to by the "vp". Recompute
 * it if necessary. Also set the approximate redisplay
 * cost. The validity of the hash code is marked by
 * a flag bit. The cost understand the advantages
 * of erase to end of line. Tuned for the VAX
 * by Bob McNamara; better than it used to be on
 * just about any machine.
 */
hash(vp)
register VIDEO	*vp;
{
	register int	i;
	register int	n;
	register char	*s;
 
	if ((vp->v_flag&VFHBAD) != 0) {		/* Hash bad.		*/
		s = &vp->v_text[ncol-1];
		for (i=ncol; i!=0; --i, --s)
			if (*s != ' ')
				break;
		n = ncol-i;			/* Erase cheaper?	*/
		if (n > tceeol)
			n = tceeol;
		vp->v_cost = i+n;		/* Bytes + blanks.	*/
		for (n=0; i!=0; --i, --s)
			n = (n<<5) + n + *s;
		vp->v_hash = n;			/* Hash code.		*/
		vp->v_flag &= ~VFHBAD;		/* Flag as all done.	*/
	}
}

/*
 * Compute the Insert-Delete
 * cost matrix. The dynamic programming algorithm
 * described by James Gosling is used. This code assumes
 * that the line above the echo line is the last line involved
 * in the scroll region. This is easy to arrange on the VT100
 * because of the scrolling region. The "offs" is the origin 0
 * offset of the first row in the virtual/physical screen that
 * is being updated; the "size" is the length of the chunk of
 * screen being updated. For a full screen update, use offs=0
 * and size=nrow-1.
 *
 * Older versions of this code implemented the score matrix by
 * a two dimensional array of SCORE nodes. This put all kinds of
 * multiply instructions in the code! This version is written to
 * use a linear array and pointers, and contains no multiplication
 * at all. The code has been carefully looked at on the VAX, with
 * only marginal checking on other machines for efficiency. In
 * fact, this has been tuned twice! Bob McNamara tuned it even
 * more for the VAX, which is a big issue for him because of
 * the 66 line X displays.
 *
 * On some machines, replacing the "for (i=1; i<=size; ++i)" with
 * i = 1; do { } while (++i <=size)" will make the code quite a
 * bit better; but it looks ugly.
 */
setscores(offs, size)
{
	register SCORE	*sp;
	register int	tempcost;
	register int	bestcost;
	register int	j;
	register int	i;
	register VIDEO	**vp;
	register VIDEO	**pp;
	register SCORE	*sp1;
	register VIDEO	**vbase;
	register VIDEO	**pbase;
 
	vbase = &vscreen[offs-1];		/* By hand CSE's.	*/
	pbase = &pscreen[offs-1];
	score[0].s_itrace = 0;			/* [0, 0]		*/
	score[0].s_jtrace = 0;
	score[0].s_cost   = 0;
	sp = &score[1];				/* Row 0, inserts.	*/
	tempcost = 0;
	vp = &vbase[1];
	for (j=1; j<=size; ++j) {
		sp->s_itrace = 0;
		sp->s_jtrace = j-1;
		tempcost += tcinsl;
		tempcost += (*vp)->v_cost;
		sp->s_cost = tempcost;
		++vp;
		++sp;
	}
	sp = &score[NROW];			/* Column 0, deletes.	*/
	tempcost = 0;
	for (i=1; i<=size; ++i) {
		sp->s_itrace = i-1;
		sp->s_jtrace = 0;
		tempcost  += tcdell;
		sp->s_cost = tempcost;
		sp += NROW;
	}
	sp1 = &score[NROW+1];			/* [1, 1].		*/
	pp = &pbase[1];
	for (i=1; i<=size; ++i) {
		sp = sp1;
		vp = &vbase[1];
		for (j=1; j<=size; ++j) {
			sp->s_itrace = i-1;
			sp->s_jtrace = j;
			bestcost = (sp-NROW)->s_cost;
			if (j != size)		/* Cd(A[i])=0 @ Dis.	*/
				bestcost += tcdell;
			tempcost = (sp-1)->s_cost;
			tempcost += (*vp)->v_cost;
			if (i != size)		/* Ci(B[j])=0 @ Dsj.	*/
				tempcost += tcinsl;
			if (tempcost < bestcost) {
				sp->s_itrace = i;
				sp->s_jtrace = j-1;
				bestcost = tempcost;
			}
			tempcost = (sp-NROW-1)->s_cost;
			if ((*pp)->v_color != (*vp)->v_color
			||  (*pp)->v_hash  != (*vp)->v_hash)
				tempcost += (*vp)->v_cost;
			if (tempcost < bestcost) {
				sp->s_itrace = i-1;
				sp->s_jtrace = j-1;
				bestcost = tempcost;
			}
			sp->s_cost = bestcost;
			++sp;			/* Next column.		*/
			++vp;
		}
		++pp;
		sp1 += NROW;			/* Next row.		*/
	}
}

/*
 * Trace back through the dynamic programming cost
 * matrix, and update the screen using an optimal sequence
 * of redraws, insert lines, and delete lines. The "offs" is
 * the origin 0 offset of the chunk of the screen we are about to
 * update. The "i" and "j" are always started in the lower right
 * corner of the matrix, and imply the size of the screen.
 * A full screen traceback is called with offs=0 and i=j=nrow-1.
 * There is some do-it-yourself double subscripting here,
 * which is acceptable because this routine is much less compute
 * intensive then the code that builds the score matrix!
 */
traceback(offs, size, i, j)
{
	register int	itrace;
	register int	jtrace;
	register int	k;
	register int	ninsl;
	register int	ndraw;
	register int	ndell;

	if (i==0 && j==0)			/* End of update.	*/
		return;
	itrace = score[(NROW*i) + j].s_itrace;
	jtrace = score[(NROW*i) + j].s_jtrace;
	if (itrace == i) {			/* [i, j-1]		*/
		ninsl = 0;			/* Collect inserts.	*/
		if (i != size)
			ninsl = 1;
		ndraw = 1;
		while (itrace!=0 || jtrace!=0) {
			if (score[(NROW*itrace) + jtrace].s_itrace != itrace)
				break;
			jtrace = score[(NROW*itrace) + jtrace].s_jtrace;
			if (i != size)
				++ninsl;
			++ndraw;
		}
		traceback(offs, size, itrace, jtrace);
		if (ninsl != 0) {
			ttcolor(CTEXT);
			ttinsl(offs+j-ninsl, offs+size-1, ninsl);
		}
		do {				/* B[j], A[j] blank.	*/
			k = offs+j-ndraw;
			uline(k, vscreen[k], &blanks);
		} while (--ndraw);
		return;
	}
	if (jtrace == j) {			/* [i-1, j]		*/
		ndell = 0;			/* Collect deletes.	*/
		if (j != size)
			ndell = 1;
		while (itrace!=0 || jtrace!=0) {
			if (score[(NROW*itrace) + jtrace].s_jtrace != jtrace)
				break;
			itrace = score[(NROW*itrace) + jtrace].s_itrace;
			if (j != size)
				++ndell;
		}
		if (ndell != 0) {
			ttcolor(CTEXT);
			ttdell(offs+i-ndell, offs+size-1, ndell);
		}
		traceback(offs, size, itrace, jtrace);
		return;
	}
	traceback(offs, size, itrace, jtrace);
	k = offs+j-1;
	uline(k, vscreen[k], pscreen[offs+i-1]);
}
#endif
