/*
 * Name:	MicroEMACS
 * 		File commands.
 * Version:	29
 * Last edit:	05-Feb-86
 * By:		rex::conroy
 *		decvax!decwrl!dec-rhea!dec-rex!conroy
 */
#include	"def.h"

/*
 * Read a file into the current
 * buffer. This is really easy; all you do it
 * find the name of the file, and call the standard
 * "read a file into the current buffer" code.
 */
fileread(f, n, k)
{
	register int	s;
	char		fname[NFILEN];

	if ((s=ereply("Read file: ", fname, NFILEN)) != TRUE)
		return (s);
	adjustcase(fname);
	return (readin(fname));
}

/*
 * Select a file for editing.
 * Look around to see if you can find the
 * fine in another buffer; if you can find it
 * just switch to the buffer. If you cannot find
 * the file, create a new buffer, read in the
 * text, and switch to the new buffer.
 */
filevisit(f, n, k)
{
	register BUFFER	*bp;
	register WINDOW	*wp;
	register LINE	*lp;
	register int	i;
	register int	s;
	char		bname[NBUFN];
	char		fname[NFILEN];

	if ((s=ereply("Visit file: ", fname, NFILEN)) != TRUE)
		return (s);
	adjustcase(fname);
	for (bp=bheadp; bp!=NULL; bp=bp->b_bufp) {
		if (strcmp(bp->b_fname, fname) == 0) {
			if (--curbp->b_nwnd == 0) {
				curbp->b_dotp  = curwp->w_dotp;
				curbp->b_doto  = curwp->w_doto;
				curbp->b_markp = curwp->w_markp;
				curbp->b_marko = curwp->w_marko;
			}
			curbp = bp;
			curwp->w_bufp  = bp;
			if (bp->b_nwnd++ == 0) {
				curwp->w_dotp  = bp->b_dotp;
				curwp->w_doto  = bp->b_doto;
				curwp->w_markp = bp->b_markp;
				curwp->w_marko = bp->b_marko;
			} else {
				wp = wheadp;
				while (wp != NULL) {
					if (wp!=curwp && wp->w_bufp==bp) {
						curwp->w_dotp  = wp->w_dotp;
						curwp->w_doto  = wp->w_doto;
						curwp->w_markp = wp->w_markp;
						curwp->w_marko = wp->w_marko;
						break;
					}
					wp = wp->w_wndp;
				}
			}
			lp = curwp->w_dotp;
			i = curwp->w_ntrows/2;
			while (i-- && lback(lp)!=curbp->b_linep)
				lp = lback(lp);
			curwp->w_linep = lp;
			curwp->w_flag |= WFMODE|WFHARD;
			if (kbdmop == NULL)
				eprintf("[Old buffer]");
			return (TRUE);
		}
	}
	makename(bname, fname);			/* New buffer name.	*/
	while ((bp=bfind(bname, FALSE)) != NULL) {
		s = ereply("Buffer name: ", bname, NBUFN);
		if (s == ABORT)			/* ^G to just quit	*/
			return (s);
		if (s == FALSE) {		/* CR to clobber it	*/
			makename(bname, fname);
			break;
		}
	}
	if (bp==NULL && (bp=bfind(bname, TRUE))==NULL) {
		eprintf("Cannot create buffer");
		return (FALSE);
	}
	if (--curbp->b_nwnd == 0) {		/* Undisplay.		*/
		curbp->b_dotp = curwp->w_dotp;
		curbp->b_doto = curwp->w_doto;
		curbp->b_markp = curwp->w_markp;
		curbp->b_marko = curwp->w_marko;
	}
	curbp = bp;				/* Switch to it.	*/
	curwp->w_bufp = bp;
	curbp->b_nwnd++;
	return (readin(fname));			/* Read it in.		*/
}

/*
 * Read the file "fname" into the current buffer.
 * Make all of the text in the buffer go away, after checking
 * for unsaved changes. This is called by the "read" command, the
 * "visit" command, and the mainline (for "uemacs file"). If the
 * BACKUP conditional is set, then this routine also does the read
 * end of backup processing. The BFBAK flag, if set in a buffer,
 * says that a backup should be taken. It is set when a file is
 * read in, but not on a new file (you don't need to make a backup
 * copy of nothing). Return a standard status. Print a summary
 * (lines read, error message) out as well.
 */
readin(fname)
char	fname[];
{
	register LINE	*lp1;
	register LINE	*lp2;
	register int	i;
	register WINDOW	*wp;
	register BUFFER	*bp;
	register int	s;
	register int	nbytes;
	register int	nline;
	char		line[NLINE];

	bp = curbp;				/* Cheap.		*/
	if ((s=bclear(bp)) != TRUE)		/* Might be old.	*/
		return (s);
#if	BACKUP
	bp->b_flag &= ~(BFCHG|BFBAK);		/* No change, backup.	*/
#else
	bp->b_flag &= ~BFCHG;			/* No change.		*/
#endif
	strcpy(bp->b_fname, fname);
	if ((s=ffropen(fname)) == FIOERR) 	/* Hard file open.	*/
		goto out;
	if (s == FIOFNF) {			/* File not found.	*/
		if (kbdmop == NULL)
			eprintf("[New file]");
		goto out;
	}
	nline = 0;
	while ((s=ffgetline(line, NLINE)) == FIOSUC) {
		nbytes = strlen(line);
		if ((lp1=lalloc(nbytes)) == NULL) {
			s = FIOERR;		/* Keep message on the	*/
			break;			/* display.		*/
		}
		lp2 = lback(curbp->b_linep);
		lp2->l_fp = lp1;
		lp1->l_fp = curbp->b_linep;
		lp1->l_bp = lp2;
		curbp->b_linep->l_bp = lp1;
		for (i=0; i<nbytes; ++i)
			lputc(lp1, i, line[i]);
		++nline;
	}
	ffclose();				/* Ignore errors.	*/
	if (s==FIOEOF && kbdmop==NULL) {	/* Don't zap an error.	*/
		if (nline == 1)
			eprintf("[Read 1 line]");
		else
			eprintf("[Read %d lines]", nline);
	}
#if	BACKUP
	curbp->b_flag |= BFBAK;			/* Need a backup.	*/
#endif
out:
	for (wp=wheadp; wp!=NULL; wp=wp->w_wndp) {
		if (wp->w_bufp == curbp) {
			wp->w_linep = lforw(curbp->b_linep);
			wp->w_dotp  = lforw(curbp->b_linep);
			wp->w_doto  = 0;
			wp->w_markp = NULL;
			wp->w_marko = 0;
			wp->w_flag |= WFMODE|WFHARD;
		}
	}
	if (s == FIOERR)			/* False if error.	*/
		return (FALSE);
	return (TRUE);
}

/*
 * Take a file name, and from it
 * fabricate a buffer name. This routine knows
 * about the syntax of file names on the target system.
 * BDC1		left scan delimiter.
 * BDC2		optional second left scan delimiter.
 * BDC3		optional right scan delimiter.
 */
makename(bname, fname)
char	bname[];
char	fname[];
{
	register char	*cp1;
	register char	*cp2;

	cp1 = &fname[0];
	while (*cp1 != 0)
		++cp1;
#ifdef	BDC2
	while (cp1!=&fname[0] && cp1[-1]!=BDC1 && cp1[-1]!=BDC2)
		--cp1;
#else
	while (cp1!=&fname[0] && cp1[-1]!=BDC1)
		--cp1;
#endif
	cp2 = &bname[0];
#ifdef	BDC3
	while (cp2!=&bname[NBUFN-1] && *cp1!=0 && *cp1!=BDC3)
		*cp2++ = *cp1++;
#else
	while (cp2!=&bname[NBUFN-1] && *cp1!=0)
		*cp2++ = *cp1++;
#endif
	*cp2 = 0;
}

/*
 * Ask for a file name, and write the
 * contents of the current buffer to that file.
 * Update the remembered file name and clear the
 * buffer changed flag. This handling of file names
 * is different from the earlier versions, and
 * is more compatable with Gosling EMACS than
 * with ITS EMACS.
 */
filewrite(f, n, k)
{
	register WINDOW	*wp;
	register int	s;
	char		fname[NFILEN];

	if ((s=ereply("Write file: ", fname, NFILEN)) != TRUE)
		return (s);
	adjustcase(fname);
	if ((s=writeout(fname)) == TRUE) {
		strcpy(curbp->b_fname, fname);
		curbp->b_flag &= ~BFCHG;
		wp = wheadp;			/* Update mode lines.	*/
		while (wp != NULL) {
			if (wp->w_bufp == curbp)
				wp->w_flag |= WFMODE;
			wp = wp->w_wndp;
		}
	}
#if	BACKUP
	curbp->b_flag &= ~BFBAK;		/* No backup.		*/
#endif
	return (s);
}

/*
 * Save the contents of the current buffer back into
 * its associated file. Do nothing if there have been no changes
 * (is this a bug, or a feature). Error if there is no remembered
 * file name. If this is the first write since the read or visit,
 * then a backup copy of the file is made.
 */
filesave(f, n, k)
{
	register WINDOW	*wp;
	register int	s;

	if ((curbp->b_flag&BFCHG) == 0)		/* Return, no changes.	*/
		return (TRUE);
	if (curbp->b_fname[0] == 0) {		/* Must have a name.	*/
		eprintf("No file name");
		return (FALSE);
	}
#if	BACKUP
	if ((curbp->b_flag&BFBAK) != 0) {
		s = fbackupfile(curbp->b_fname);
		if (s == ABORT)			/* Hard error.		*/
			return (s);
		if (s == FALSE			/* Softer error.	*/
		&& (s=eyesno("Backup error, save anyway")) != TRUE)
			return (s);
	}
#endif
	if ((s=writeout(curbp->b_fname)) == TRUE) {
		curbp->b_flag &= ~BFCHG;
		wp = wheadp;			/* Update mode lines.	*/
		while (wp != NULL) {
			if (wp->w_bufp == curbp)
				wp->w_flag |= WFMODE;
			wp = wp->w_wndp;
		}
	}
#if	BACKUP
	curbp->b_flag &= ~BFBAK;		/* No backup.		*/
#endif
	return (s);
}

/*
 * This function performs the details of file
 * writing. Uses the file management routines in the
 * "fileio.c" package. The number of lines written is
 * displayed. Sadly, it looks inside a LINE; provide
 * a macro for this. Most of the grief is error
 * checking of some sort.
 */
writeout(fn)
char	*fn;
{
	register int	s;
	register LINE	*lp;
	register int	nline;

	if ((s=ffwopen(fn)) != FIOSUC)		/* Open writes message.	*/
		return (FALSE);
	lp = lforw(curbp->b_linep);		/* First line.		*/
	nline = 0;				/* Number of lines.	*/
	while (lp != curbp->b_linep) {
		if ((s=ffputline(&lp->l_text[0], llength(lp))) != FIOSUC)
			break;
		++nline;
		lp = lforw(lp);
	}
	if (s == FIOSUC) {			/* No write error.	*/
		s = ffclose();
		if (s==FIOSUC && kbdmop==NULL) {
			if (nline == 1)
				eprintf("[Wrote 1 line]");
			else
				eprintf("[Wrote %d lines]", nline);
		}
	} else					/* Ignore close error	*/
		ffclose();			/* if a write error.	*/
	if (s != FIOSUC)			/* Some sort of error.	*/
		return (FALSE);
	return (TRUE);
}

/*
 * The command allows the user
 * to modify the file name associated with
 * the current buffer. It is like the "f" command
 * in UNIX "ed". The operation is simple; just zap
 * the name in the BUFFER structure, and mark the windows
 * as needing an update. You can type a blank line at the
 * prompt if you wish.
 */
filename(f, n, k)
{
	register WINDOW	*wp;
	register int	s;
	char	 	fname[NFILEN];

	if ((s=ereply("Name: ", fname, NFILEN)) == ABORT)
		return (s);
	adjustcase(fname);
	strcpy(curbp->b_fname, fname);		/* Fix name.		*/
	wp = wheadp;				/* Update mode lines.	*/
	while (wp != NULL) {
		if (wp->w_bufp == curbp)
			wp->w_flag |= WFMODE;
		wp = wp->w_wndp;
	}
#if	BACKUP
	curbp->b_flag &= ~BFBAK;		/* No backup.		*/
#endif
	return (TRUE);
}
