/*
 * Name:	MicroEMACS
 *		Atari 520ST file I/O.
 * Version:	30
 * Last edit:	22-Feb-86
 * By:		rex::conroy
 *		decvax!decwrl!dec-rhea!dec-rex!conroy
 */
#include	"def.h"

extern	FILE	*fopen();
static	FILE	*ffp;

/*
 * Open a file for reading.
 */
ffropen(fn)
char	*fn;
{
	ffp = fopen(fn, "r");
	adjustcase(fn);
	if (ffp == NULL)
		return (FIOFNF);
	return (FIOSUC);
}

/*
 * Open a file for writing.
 * Return TRUE if all is well, and
 * FALSE on error (cannot create).
 */
ffwopen(fn)
char	*fn;
{
	ffp = fopen(fn, "w");
	adjustcase(fn);
	if (ffp == NULL) {
		eprintf("Cannot open file for writing");
		return (FIOERR);
	}
	return (FIOSUC);
}

/*
 * Close a file.
 * Should look at the status.
 */
ffclose()
{
	fclose(ffp);
	return (FIOSUC);
}

/*
 * Write a line to the already
 * opened file. The "buf" points to the
 * buffer, and the "nbuf" is its length, less
 * the free newline. Return the status.
 * Check only at the newline.
 */
ffputline(buf, nbuf)
register char	buf[];
{
	register int	i;

	for (i=0; i<nbuf; ++i)
		putc(buf[i]&0xFF, ffp);
	putc('\n', ffp);
	if (ferror(ffp) != FALSE) {
		eprintf("Write I/O error");
		return (FIOERR);
	}
	return (FIOSUC);
}

/*
 * Read a line from a file, and store the bytes
 * in the supplied buffer. Stop on end of file or end of
 * line. Don't get upset by files that don't have an end of
 * line on the last line; this seem to be common on CP/M-86 and
 * MS-DOS (the suspected culprit is VAX/VMS kermit, but this
 * has not been confirmed. If this is sufficiently researched
 * it may be possible to pull this kludge). Delete any CR
 * followed by an LF. This is mainly for runoff documents,
 * both on VMS and on Ultrix (they get copied over from
 * VMS systems with DECnet).
 */
ffgetline(buf, nbuf)
register char	buf[];
{
	register int	c;
	register int	i;

	i = 0;
	for (;;) {
		c = getc(ffp);
		if (c == '\r') {		/* Delete any non-stray	*/
			c = getc(ffp);		/* carriage returns.	*/
			if (c != '\n') {
				if (i >= nbuf-1) {
					eprintf("File has long line");
					return (FIOERR);
				}
				buf[i++] = '\r';
			}
		}
		if (c==EOF || c=='\n')		/* End of line.		*/
			break;
		if (i >= nbuf-1) {
			eprintf("File has long line");
			return (FIOERR);
		}
		buf[i++] = c;
	}
	if (c == EOF) {				/* End of file.		*/
		if (ferror(ffp) != FALSE) {
			eprintf("File read error");
			return (FIOERR);
		}
		if (i == 0)			/* Don't get upset if	*/
			return (FIOEOF);	/* no newline at EOF.	*/
	}
	buf[i] = 0;
	return (FIOSUC);
}

/*
 * Finish this routine when you decide
 * what the right thing to do when renaming a
 * file for backup purposes.
 */
fbackupfile(fname)
char	*fname;
{
	return (TRUE);
}

/*
 * Zap file name to lower case, since
 * the system on the 520ST has case insensitive
 * file names, and lower looks better in the
 * modelines than upper.
 */
adjustcase(fn)
register char	*fn;
{
	register int	c;

	while ((c = *fn) != 0) {
		if (ISUPPER(c) != FALSE)
			*fn = TOLOWER(c);
		++fn;
	}
}
