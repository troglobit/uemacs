/*
 * Name:	MicroEMACS
 *		Word mode commands.
 * Version:	29
 * Last edit:	05-Feb-86
 * By:		rex::conroy
 *		decvax!decwrl!dec-rhea!dec-rex!conroy
 *
 * The routines in this file
 * implement commands that work word at
 * a time. There are all sorts of word mode
 * commands. If I do any sentence and/or paragraph
 * mode commands, they are likely to be put in
 * this file.
 */
#include	"def.h"

/*
 * Move the cursor backward by
 * "n" words. All of the details of motion
 * are performed by the "backchar" and "forwchar"
 * routines. Error if you try to move beyond
 * the buffers.
 */
backword(f, n, k)
{
	if (n < 0)
		return (forwword(f, -n, KRANDOM));
	if (backchar(FALSE, 1, KRANDOM) == FALSE)
		return (FALSE);
	while (n--) {
		while (inword() == FALSE) {
			if (backchar(FALSE, 1, KRANDOM) == FALSE)
				return (FALSE);
		}
		while (inword() != FALSE) {
			if (backchar(FALSE, 1, KRANDOM) == FALSE)
				return (FALSE);
		}
	}
	return (forwchar(FALSE, 1, KRANDOM));
}

/*
 * Move the cursor forward by
 * the specified number of words. All of the
 * motion is done by "forwchar". Error if you
 * try and move beyond the buffer's end.
 */
forwword(f, n, k)
{
	if (n < 0)
		return (backword(f, -n, KRANDOM));
	while (n--) {
		while (inword() == FALSE) {
			if (forwchar(FALSE, 1, KRANDOM) == FALSE)
				return (FALSE);
		}
		while (inword() != FALSE) {
			if (forwchar(FALSE, 1, KRANDOM) == FALSE)
				return (FALSE);
		}
	}
	return (TRUE);
}

/*
 * Move the cursor forward by
 * the specified number of words. As you move,
 * convert any characters to upper case. Error
 * if you try and move beyond the end of the
 * buffer.
 */
upperword(f, n, k)
{
	register int	c;

	if (n < 0)
		return (FALSE);
	while (n--) {
		while (inword() == FALSE) {
			if (forwchar(FALSE, 1, KRANDOM) == FALSE)
				return (FALSE);
		}
		while (inword() != FALSE) {
			c = lgetc(curwp->w_dotp, curwp->w_doto);
			if (ISLOWER(c) != FALSE) {
				c = TOUPPER(c);
				lputc(curwp->w_dotp, curwp->w_doto, c);
				lchange(WFHARD);
			}
			if (forwchar(FALSE, 1, KRANDOM) == FALSE)
				return (FALSE);
		}
	}
	return (TRUE);
}

/*
 * Move the cursor forward by
 * the specified number of words. As you move
 * convert characters to lower case. Error if you
 * try and move over the end of the buffer.
 */
lowerword(f, n, k)
{
	register int	c;

	if (n < 0)
		return (FALSE);
	while (n--) {
		while (inword() == FALSE) {
			if (forwchar(FALSE, 1, KRANDOM) == FALSE)
				return (FALSE);
		}
		while (inword() != FALSE) {
			c = lgetc(curwp->w_dotp, curwp->w_doto);
			if (ISUPPER(c) != FALSE) {
				c = TOLOWER(c);
				lputc(curwp->w_dotp, curwp->w_doto, c);
				lchange(WFHARD);
			}
			if (forwchar(FALSE, 1, KRANDOM) == FALSE)
				return (FALSE);
		}
	}
	return (TRUE);
}

/*
 * Move the cursor forward by
 * the specified number of words. As you move
 * convert the first character of the word to upper
 * case, and subsequent characters to lower case. Error
 * if you try and move past the end of the buffer.
 */
capword(f, n, k)
{
	register int	c;

	if (n < 0)
		return (FALSE);
	while (n--) {
		while (inword() == FALSE) {
			if (forwchar(FALSE, 1, KRANDOM) == FALSE)
				return (FALSE);
		}
		if (inword() != FALSE) {
			c = lgetc(curwp->w_dotp, curwp->w_doto);
			if (ISLOWER(c) != FALSE) {
				c = TOUPPER(c);
				lputc(curwp->w_dotp, curwp->w_doto, c);
				lchange(WFHARD);
			}
			if (forwchar(FALSE, 1, KRANDOM) == FALSE)
				return (FALSE);
			while (inword() != FALSE) {
				c = lgetc(curwp->w_dotp, curwp->w_doto);
				if (ISUPPER(c) != FALSE) {
					c = TOLOWER(c);
					lputc(curwp->w_dotp, curwp->w_doto, c);
					lchange(WFHARD);
				}
				if (forwchar(FALSE, 1, KRANDOM) == FALSE)
					return (FALSE);
			}
		}
	}
	return (TRUE);
}

/*
 * Kill forward by "n" words. The rules for final
 * status are now different. It is not considered an error
 * to delete fewer words than you asked. This lets you say
 * "kill lots of words" and have the command stop in a reasonable
 * way when it hits the end of the buffer. Normally this is
 */
delfword(f, n, k)
{
	register int	size;
	register LINE	*dotp;
	register int	doto;

	if (n < 0)
		return (FALSE);
	if ((lastflag&CFKILL) == 0)		/* Purge kill buffer.	*/
		kdelete();
	thisflag |= CFKILL;
	dotp = curwp->w_dotp;
	doto = curwp->w_doto;
	size = 0;
	while (n--) {
		while (inword() == FALSE) {
			if (forwchar(FALSE, 1, KRANDOM) == FALSE)
				goto out;	/* Hit end of buffer.	*/
			++size;
		}
		while (inword() != FALSE) {
			if (forwchar(FALSE, 1, KRANDOM) == FALSE)
				goto out;	/* Hit end of buffer.	*/
			++size;
		}
	}
out:
	curwp->w_dotp = dotp;
	curwp->w_doto = doto;
	return (ldelete(size, TRUE));
}

/*
 * Kill backwards by "n" words. The rules
 * for success and failure are now different, to prevent
 * strange behavior at the start of the buffer. The command
 * only fails if something goes wrong with the actual delete
 * of the characters. It is successful even if no characters
 * are deleted, or if you say delete 5 words, and there are
 * only 4 words left. I considered making the first call
 * to "backchar" special, but decided that that would just
 * be wierd. Normally this is bound to "M-Rubout" and
 * to "M-Backspace".
 */
delbword(f, n, k)
{
	register int	size;

	if (n < 0)
		return (FALSE);
	if ((lastflag&CFKILL) == 0)		/* Purge kill buffer.	*/
		kdelete();
	thisflag |= CFKILL;
	if (backchar(FALSE, 1, KRANDOM) == FALSE)
		return (TRUE);			/* Hit buffer start.	*/
	size = 1;				/* One deleted.		*/
	while (n--) {
		while (inword() == FALSE) {
			if (backchar(FALSE, 1, KRANDOM) == FALSE)
				goto out;	/* Hit buffer start.	*/
			++size;
		}
		while (inword() != FALSE) {
			if (backchar(FALSE, 1, KRANDOM) == FALSE)
				goto out;	/* Hit buffer start.	*/
			++size;
		}
	}
	if (forwchar(FALSE, 1, KRANDOM) == FALSE)
		return (FALSE);
	--size;					/* Undo assumed delete.	*/
out:
	return (ldelete(size, TRUE));
}

/*
 * Return TRUE if the character at dot
 * is a character that is considered to be
 * part of a word. The word character list is hard
 * coded. Should be setable.
 */
inword()
{
	if (curwp->w_doto == llength(curwp->w_dotp))
		return (FALSE);
	if (ISWORD(lgetc(curwp->w_dotp, curwp->w_doto)) != FALSE)
		return (TRUE);
	return (FALSE);
}
