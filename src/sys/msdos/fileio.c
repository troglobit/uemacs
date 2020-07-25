/*
 * Name:	MicroEMACS
 *		MS-DOS file I/O.
 * Version:	29
 * Last edit:	05-Feb-86
 * By:		rex::conroy
 *		decvax!decwrl!dec-rhea!dec-rex!conroy
 */
#include	"def.h"

static	FILE	*ffp;

/*
 * Open a file for reading.
 */
ffropen(fn)
char	*fn;
{
	if ((ffp=fopen(fn, "r")) == NULL)
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
	if ((ffp=fopen(fn, "w")) == NULL) {
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
 * Some backup user on MS-DOS might want
 * to determine some rule for doing backups on that
 * system, and fix this. I don't use MS-DOS, so I don't
 * know what the right rules would be. Return TRUE so
 * the caller does not abort a write.
 */
fbackupfile(fname)
char	*fname;
{
	return (TRUE);				/* Hack.		*/
}

/*
 * The string "fn" is a file name.
 * Perform any required case adjustments. All sustems
 * we deal with so far have case insensitive file systems.
 * We zap everything to lower case. The problem we are trying
 * to solve is getting 2 buffers holding the same file if
 * you visit one of them with the "caps lock" key down.
 * On UNIX file names are dual case, so we leave
 * everything alone.
 */
adjustcase(fn)
register char	*fn;
{
	register int	c;

	while ((c = *fn) != 0) {
		if (c>='A' && c<='Z')
			*fn = c + 'a' - 'A';
		++fn;
	}
}
