/*
 * Name:	MicroEMACS
 *		Region based commands.
 * Version:	29
 * Last edit:	12-Feb-86
 * By:		rex::conroy
 *		decvax!decwrl!dec-rhea!dec-rex!conroy
 * What:	Region operations.
 *
 * The routines in this file
 * deal with the region, that magic space
 * between "." and mark. Some functions are
 * commands. Some functions are just for
 * internal use.
 */
#include	"def.h"

/*
 * Kill the region. Ask "getregion"
 * to figure out the bounds of the region.
 * Move "." to the start, and kill the characters.
 */
killregion(f, n, k)
{
	register int	s;
	REGION		region;

	if ((s=getregion(&region)) != TRUE)
		return (s);
	if ((lastflag&CFKILL) == 0)		/* This is a kill type	*/
		kdelete();			/* command, so do magic	*/
	thisflag |= CFKILL;			/* kill buffer stuff.	*/
	curwp->w_dotp = region.r_linep;
	curwp->w_doto = region.r_offset;
	return (ldelete(region.r_size, TRUE));
}

/*
 * Copy all of the characters in the
 * region to the kill buffer. Don't move dot
 * at all. This is a bit like a kill region followed
 * by a yank.
 */
copyregion(f, n, k)
{
	register LINE	*linep;
	register int	loffs;
	register int	s;
	REGION		region;

	if ((s=getregion(&region)) != TRUE)
		return (s);
	if ((lastflag&CFKILL) == 0)		/* Kill type command.	*/
		kdelete();
	thisflag |= CFKILL;
	linep = region.r_linep;			/* Current line.	*/
	loffs = region.r_offset;		/* Current offset.	*/
	while (region.r_size--) {
		if (loffs == llength(linep)) {	/* End of line.		*/
			if ((s=kinsert('\n')) != TRUE)
				return (s);
			linep = lforw(linep);
			loffs = 0;
		} else {			/* Middle of line.	*/
			if ((s=kinsert(lgetc(linep, loffs))) != TRUE)
				return (s);
			++loffs;
		}
	}
	return (TRUE);
}

/*
 * Lower case region. Zap all of the upper
 * case characters in the region to lower case. Use
 * the region code to set the limits. Scan the buffer,
 * doing the changes. Call "lchange" to ensure that
 * redisplay is done in all buffers. 
 */
lowerregion(f, n, k)
{
	register LINE	*linep;
	register int	loffs;
	register int	c;
	register int	s;
	REGION		region;

	if ((s=getregion(&region)) != TRUE)
		return (s);
	lchange(WFHARD);
	linep = region.r_linep;
	loffs = region.r_offset;
	while (region.r_size--) {
		if (loffs == llength(linep)) {
			linep = lforw(linep);
			loffs = 0;
		} else {
			c = lgetc(linep, loffs);
			if (ISUPPER(c) != FALSE)
				lputc(linep, loffs, TOLOWER(c));
			++loffs;
		}
	}
	return (TRUE);
}

/*
 * Upper case region. Zap all of the lower
 * case characters in the region to upper case. Use
 * the region code to set the limits. Scan the buffer,
 * doing the changes. Call "lchange" to ensure that
 * redisplay is done in all buffers. 
 */
upperregion(f, n, k)
{
	register LINE	*linep;
	register int	loffs;
	register int	c;
	register int	s;
	REGION		region;

	if ((s=getregion(&region)) != TRUE)
		return (s);
	lchange(WFHARD);
	linep = region.r_linep;
	loffs = region.r_offset;
	while (region.r_size--) {
		if (loffs == llength(linep)) {
			linep = lforw(linep);
			loffs = 0;
		} else {
			c = lgetc(linep, loffs);
			if (ISLOWER(c) != FALSE)
				lputc(linep, loffs, TOUPPER(c));
			++loffs;
		}
	}
	return (TRUE);
}

/*
 * This routine figures out the bound of the region
 * in the current window, and stores the results into the fields
 * of the REGION structure. Dot and mark are usually close together,
 * but I don't know the order, so I scan outward from dot, in both
 * directions, looking for mark. The size is kept in a long. At the
 * end, after the size is figured out, it is assigned to the size
 * field of the region structure. If this assignment loses any bits,
 * then we print an error. This is "type independent" overflow
 * checking. All of the callers of this routine should be ready to
 * get an ABORT status, because I might add a "if regions is big,
 * ask before clobberring" flag.
 */
getregion(rp)
register REGION	*rp;
{
	register LINE	*flp;
	register LINE	*blp;
	register long	fsize;			/* Long now.		*/
	register long	bsize;

	if (curwp->w_markp == NULL) {
		eprintf("No mark in this window");
		return (FALSE);
	}
	if (curwp->w_dotp == curwp->w_markp) {	/* "r_size" always ok.	*/
		rp->r_linep = curwp->w_dotp;
		if (curwp->w_doto < curwp->w_marko) {
			rp->r_offset = curwp->w_doto;
			rp->r_size = curwp->w_marko-curwp->w_doto;
		} else {
			rp->r_offset = curwp->w_marko;
			rp->r_size = curwp->w_doto-curwp->w_marko;
		}
		return (TRUE);
	}
	blp = curwp->w_dotp;			/* Get region size.	*/
	flp = curwp->w_dotp;
	bsize = curwp->w_doto;
	fsize = llength(flp)-curwp->w_doto+1;
	while (flp!=curbp->b_linep || lback(blp)!=curbp->b_linep) {
		if (flp != curbp->b_linep) {
			flp = lforw(flp);
			if (flp == curwp->w_markp) {
				rp->r_linep = curwp->w_dotp;
				rp->r_offset = curwp->w_doto;
				return (setsize(rp, fsize+curwp->w_marko));
			}
			fsize += llength(flp)+1;
		}
		if (lback(blp) != curbp->b_linep) {
			blp = lback(blp);
			bsize += llength(blp)+1;
			if (blp == curwp->w_markp) {
				rp->r_linep = blp;
				rp->r_offset = curwp->w_marko;
				return (setsize(rp, bsize-curwp->w_marko));
			}
		}
	}
	eprintf("Bug: lost mark");		/* Gak!			*/
	return (FALSE);
}

/*
 * Set size, and check for overflow.
 */
setsize(rp, size)
register REGION	*rp;
register long	size;
{
	rp->r_size = size;
	if (rp->r_size != size) {
		eprintf("Region is too large");
		return (FALSE);
	}
	return (TRUE);
}
