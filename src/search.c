/*
 * Name:	MicroEMACS
 * 		Search commands.
 * Version:	30
 * Last edit:	14-Feb-86
 * By:		rex::conroy, rex::ellison
 *		decvax!decwrl!dec-rhea!dec-rex!conroy
 *		                   ...!dec-vox!ellison
 *
 * The functions in this file implement the
 * search commands (both plain and incremental searches
 * are supported) and the query-replace command.
 *
 * The plain old search code is part of the original
 * MicroEMACS "distribution". The incremental search code,
 * and the query-replace code, is by Rich Ellison.
 */
#include	"def.h"

#define CCHR(x)		((x)-'@')

#define SRCH_BEGIN	(0)			/* Search sub-codes.	*/
#define	SRCH_FORW	(-1)
#define SRCH_BACK	(-2)
#define SRCH_PREV	(-3)
#define SRCH_NEXT	(-4)
#define SRCH_NOPR	(-5)
#define SRCH_ACCM	(-6)

typedef struct  {
	int	s_code;
	LINE	*s_dotp;
	int	s_doto;
}	SRCHCOM;

static	SRCHCOM	cmds[NSRCH];
static	int	cip;

int	srch_lastdir = SRCH_NOPR;		/* Last search flags.	*/

/*
 * Search forward.
 * Get a search string from the user, and search for it,
 * starting at ".". If found, "." gets moved to just after the
 * matched characters, and display does all the hard stuff.
 * If not found, it just prints a message.
 */
forwsearch(f, n, k)
{
	register int	s;

	if ((s=readpattern("Search")) != TRUE)
		return (s);
	if (forwsrch() == FALSE) {
		eprintf("Not found");
		return (FALSE);
	}
	srch_lastdir = SRCH_FORW;
	return (TRUE);
}

/*
 * Reverse search.
 * Get a search string from the  user, and search, starting at "."
 * and proceeding toward the front of the buffer. If found "." is left
 * pointing at the first character of the pattern [the last character that
 * was matched].
 */
backsearch(f, n, k)
{
	register int	s;

	if ((s=readpattern("Reverse search")) != TRUE)
		return (s);
	if (backsrch() == FALSE) {
		eprintf("Not found");
		return (FALSE);
	}
	srch_lastdir = SRCH_BACK;
	return (TRUE);
}

/* 
 * Search again, using the same search string
 * and direction as the last search command. The direction
 * has been saved in "srch_lastdir", so you know which way
 * to go.
 */
searchagain(f, n, k)
{
	if (srch_lastdir == SRCH_FORW) {
		if (forwsrch() == FALSE) {
			eprintf("Not found");
			return (FALSE);
		}
		return (TRUE);
	}
	if (srch_lastdir == SRCH_BACK) {
		if (backsrch() == FALSE) {
			eprintf("Not found");
			return (FALSE);
		}
		return (TRUE);
	}
	eprintf("No last search");
	return (FALSE);
}

/*
 * Use incremental searching, initially in the forward direction.
 * isearch ignores any explicit arguments.
 */
forwisearch(f, n, k)
{
	return (isearch(SRCH_FORW));
}

/*
 * Use incremental searching, initially in the reverse direction.
 * isearch ignores any explicit arguments.
 */
backisearch(f, n, k)
{
	return (isearch(SRCH_BACK));
}

/*
 * Incremental Search.
 *	dir is used as the initial direction to search.
 *	^N	find next occurance  (if first thing typed reuse old string).
 *	^P	find prev occurance  (if first thing typed reuse old string).
 *	^S	switch direction to forward, find next
 *	^R	switch direction to reverse, find prev
 *	^Q	quote next character (allows searching for ^N etc.)
 *	<ESC>	exit from Isearch.
 *	<DEL>	undoes last character typed. (tricky job to do this correctly).
 *	else	accumulate into search string
 */
isearch(dir)
{
	register int	c;
	register LINE	*clp;
	register int	cbo;
	register int	success;
	int		pptr;

	for (cip=0; cip<NSRCH; cip++)
		cmds[cip].s_code = SRCH_NOPR;
	cip = 0;
	pptr = -1;
	clp = curwp->w_dotp;
	cbo = curwp->w_doto;
	is_lpush();
	is_cpush(SRCH_BEGIN);
	success = TRUE;
	is_prompt(dir, TRUE, success);
	for (;;) {
		update();
		switch (c = ttgetc()) {
		case CCHR('M'):
		case METACH:
			srch_lastdir = dir;
			eprintf("[Done]");
			return (TRUE);

		case CCHR('G'):
			curwp->w_dotp = clp;
			curwp->w_doto = cbo;
			curwp->w_flag |= WFMOVE;
			srch_lastdir = dir;
			ctrlg(FALSE, 0, KRANDOM);
			return (FALSE);

		case CCHR('S'):
		case CCHR('F'):
			if (dir == SRCH_BACK) {
				dir = SRCH_FORW;
				is_lpush();
				is_cpush(SRCH_FORW);
				success = TRUE;
			}
			/* Drop through to find next. */
		case CCHR('N'):
			if (success==FALSE && dir==SRCH_FORW)
				break;
			is_lpush();
			forwchar(FALSE, 1, KRANDOM);
			if (is_find(SRCH_NEXT) != FALSE) {
				is_cpush(SRCH_NEXT);
				pptr = strlen(pat);
			} else {
				backchar(FALSE, 1, KRANDOM);
				ttbeep();
				success = FALSE;
			}
			is_prompt(dir, FALSE, success);
			break;

		case CCHR('R'):
		case CCHR('B'):
			if (dir == SRCH_FORW) {
				dir = SRCH_BACK;
				is_lpush();
				is_cpush(SRCH_BACK);
				success = TRUE;
			}
			/* Drop through to find previous. */
		case CCHR('P'):
			if (success==FALSE && dir==SRCH_BACK)
				break;
			is_lpush();
			backchar(FALSE, 1, KRANDOM);
			if (is_find(SRCH_PREV) != FALSE) {
				is_cpush(SRCH_PREV);
				pptr = strlen(pat);
			} else {
				forwchar(FALSE, 1, KRANDOM);
				ttbeep();
				success = FALSE;
			}
			is_prompt(dir,FALSE,success);
			break;

		case 0x7F:
			if (is_undo(&pptr, &dir) != TRUE)
				return (ABORT);
			if (is_peek() != SRCH_ACCM)
				success = TRUE;
			is_prompt(dir, FALSE, success);
			break;

		case CCHR('^'):
		case CCHR('Q'):
			c = ttgetc();
		case CCHR('U'):
		case CCHR('X'):
		case CCHR('J'):
			goto  addchar;

		default:
			if (ISCTRL(c) != FALSE) {
				c += '@';
				c |= KCTRL;
				success = execute(c, FALSE, 1);
				curwp->w_flag |= WFMOVE;
				return (success);
			}				
		addchar:
			if (pptr == -1)
				pptr = 0;
			if (pptr == 0)
				success = TRUE;
			pat[pptr++] = c;
			if (pptr == NPAT) {
				eprintf("Pattern too long");
				ctrlg(FALSE, 0, KRANDOM);
				return (ABORT);
			}
			pat[pptr] = '\0';
			is_lpush();
			if (success != FALSE) {
				if (is_find(dir) != FALSE)
					is_cpush(c);
				else {
					success = FALSE;
					ttbeep();
					is_cpush(SRCH_ACCM);
				}
			} else
				is_cpush(SRCH_ACCM);
			is_prompt(dir, FALSE, success);
		}
	}
}

is_cpush(cmd)
register int	cmd;
{
	if (++cip >= NSRCH)
		cip = 0;
	cmds[cip].s_code = cmd;
}

is_lpush()
{
	register int	ctp;

	ctp = cip+1;
	if (ctp >= NSRCH)
		ctp = 0;
	cmds[ctp].s_code = SRCH_NOPR;
	cmds[ctp].s_doto = curwp->w_doto;
	cmds[ctp].s_dotp = curwp->w_dotp;
}

is_pop()
{
	if (cmds[cip].s_code != SRCH_NOPR) {
		curwp->w_doto  = cmds[cip].s_doto; 
		curwp->w_dotp  = cmds[cip].s_dotp;
		curwp->w_flag |= WFMOVE;
		cmds[cip].s_code = SRCH_NOPR;
	}
	if (--cip <= 0)
		cip = NSRCH-1;
}

is_peek()	
{
	if (cip == 0)
		return (cmds[NSRCH-1].s_code);
	else
		return (cmds[cip-1].s_code);
}

is_undo(pptr, dir)
register int	*pptr;
register int	*dir;
{
	switch (cmds[cip].s_code) {
	case SRCH_NOPR:
	case SRCH_BEGIN:
	case SRCH_NEXT:
	case SRCH_PREV:
		break;

	case SRCH_FORW:
		*dir = SRCH_BACK;
		break;

	case SRCH_BACK:
		*dir = SRCH_FORW;
		break;

	case SRCH_ACCM:
	default:
		*pptr -= 1;
		if (*pptr < 0)
			*pptr = 0;
		pat[*pptr] = '\0';
		break;
	}
	is_pop();
	return (TRUE);
}

is_find(dir)
register int	dir;
{
	register int	plen;

	plen = strlen(pat);
	if (plen != 0) {
		if (dir==SRCH_FORW || dir==SRCH_NEXT) {
			backchar(FALSE, plen, KRANDOM);
			if (forwsrch() == FALSE) {
				forwchar(FALSE, plen, KRANDOM);
				return (FALSE);
			}
			return (TRUE);
		}
		if (dir==SRCH_BACK || dir==SRCH_PREV) {
			forwchar(FALSE, plen, KRANDOM);
			if (backsrch() == FALSE) {
				backchar(FALSE, plen, KRANDOM);
				return (FALSE);
			}
			return (TRUE);
		}
		eprintf("bad call to is_find");
		ctrlg(FALSE, 0, KRANDOM);
		return (FALSE);
	}
	return (FALSE);
}

/*
 * If called with "dir" not one of SRCH_FORW
 * or SRCH_BACK, this routine used to print an error
 * message. It also used to return TRUE or FALSE,
 * depending on if it liked the "dir". However, none
 * of the callers looked at the status, so I just
 * made the checking vanish.
 */
is_prompt(dir, flag, success)
{
	if (dir == SRCH_FORW) {
		if (success != FALSE)
			is_dspl("i-search forward", flag);
		else
			is_dspl("failing i-search forward", flag);
	} else if (dir == SRCH_BACK) {
		if (success != FALSE)
			is_dspl("i-search backward", flag);
		else
			is_dspl("failing i-search backward", flag);
	}
}

/*
 * Prompt writing routine for the incremental search. 
 * The "prompt" is just a string. The "flag" determines
 * if a "[ ]" or ":" embelishment is used.
 */
is_dspl(prompt, flag)
char	*prompt;
{
	if (flag != FALSE)
		eprintf("%s [%s]", prompt, pat);
	else
		eprintf("%s: %s", prompt, pat);
}

/*
 * Query Replace.
 *	Replace strings selectively.  Does a search and replace operation.
 *	A space or a comma replaces the string, a period replaces and quits,
 *	an n doesn't replace, a C-G quits.
 */
queryrepl(f, n, k)
{
	register int	s;
	char		news[NPAT];	/* replacement string		*/
	register int	kludge;		/* Watch for saved line move	*/
	LINE		*clp;		/* saved line pointer		*/
	int		cbo;		/* offset into the saved line	*/
	int		rcnt = 0;	/* Replacements made so far	*/
	int		plen;		/* length of found string	*/

	if ((s=readpattern("Old string")) != TRUE)
		return (s);
	if ((s=ereply("New string: ",news, NPAT)) == ABORT)
		return (s);
	if (s == FALSE)
		news[0] = '\0';
	eprintf("Query Replace:  [%s] -> [%s]", pat, news);
	plen = strlen(pat);

	/*
	 * Search forward repeatedly, checking each time whether to insert
	 * or not.  The "!" case makes the check always true, so it gets put
	 * into a tighter loop for efficiency.
	 *
	 * If we change the line that is the remembered value of dot, then
	 * it is possible for the remembered value to move.  This causes great
	 * pain when trying to return to the non-existant line.
	 *
	 * possible fixes:
	 * 1) put a single, relocated marker in the WINDOW structure, handled
	 *    like mark.  The problem now becomes a what if two are needed...
	 * 2) link markers into a list that gets updated (auto structures for
	 *    the nodes)
	 * 3) Expand the mark into a stack of marks and add pushmark, popmark.
	 */

	clp = curwp->w_dotp;		/* save the return location	*/
	cbo = curwp->w_doto;
	while (forwsrch() == TRUE) {
	retry:
		update();
		switch (ttgetc()) {
		case ' ':
		case ',':
			kludge = (curwp->w_dotp == clp);
			if (lreplace(plen, news, f) == FALSE)
				return (FALSE);
			rcnt++;
			if (kludge != FALSE)
				clp = curwp->w_dotp;
			break;

		case '.':
			kludge = (curwp->w_dotp == clp);
			if (lreplace(plen, news, f) == FALSE)
				return (FALSE);
			rcnt++;
			if (kludge != FALSE)
				clp = curwp->w_dotp;
			goto stopsearch;

		case CCHR('G'):
			ctrlg(FALSE, 0, KRANDOM);
			goto stopsearch;

		case '!':
			do {
				kludge = (curwp->w_dotp == clp);
				if (lreplace(plen, news, f) == FALSE)
					return (FALSE);
				rcnt++;
				if (kludge != FALSE)
					clp = curwp->w_dotp;
			} while (forwsrch() == TRUE);
			goto stopsearch;

		case 'n':
			break;

		default:
eprintf("<SP>[,] replace, [.] rep-end, [n] don't, [!] repl rest [C-G] quit");
			goto retry;
		}
	}
stopsearch:
	curwp->w_dotp = clp;
	curwp->w_doto = cbo;
	curwp->w_flag |= WFHARD;
	update();
	if (rcnt == 0)
		eprintf("[No replacements done]");
	else if (rcnt == 1)
		eprintf("[1 replacement done]");
	else
		eprintf("[%d replacements done]", rcnt);
	return (TRUE);
}

/*
 * This routine does the real work of a
 * forward search. The pattern is sitting in the external
 * variable "pat". If found, dot is updated, the window system
 * is notified of the change, and TRUE is returned. If the
 * string isn't found, FALSE is returned.
 */
forwsrch()
{
	register LINE	*clp;
	register int	cbo;
	register LINE	*tlp;
	register int	tbo;
	register char	*pp;
	register int	c;

	clp = curwp->w_dotp;
	cbo = curwp->w_doto;
	while (clp != curbp->b_linep) {
		if (cbo == llength(clp)) {
			clp = lforw(clp);
			cbo = 0;
			c = '\n';
		} else
			c = lgetc(clp, cbo++);
		if (eq(c, pat[0]) != FALSE) {
			tlp = clp;
			tbo = cbo;
			pp  = &pat[1];
			while (*pp != 0) {
				if (tlp == curbp->b_linep)
					goto fail;
				if (tbo == llength(tlp)) {
					tlp = lforw(tlp);
					if (tlp == curbp->b_linep)
						goto fail;
					tbo = 0;
					c = '\n';
				} else
					c = lgetc(tlp, tbo++);
				if (eq(c, *pp++) == FALSE)
					goto fail;
			}
			curwp->w_dotp  = tlp;
			curwp->w_doto  = tbo;
			curwp->w_flag |= WFMOVE;
			return (TRUE);
		}
	fail:	;
	}
	return (FALSE);
}

/*
 * This routine does the real work of a
 * backward search. The pattern is sitting in the external
 * variable "pat". If found, dot is updated, the window system
 * is notified of the change, and TRUE is returned. If the
 * string isn't found, FALSE is returned.
 */
backsrch()
{
	register LINE	*clp;
	register int	cbo;
	register LINE	*tlp;
	register int	tbo;
	register int	c;
	register char	*epp;
	register char	*pp;

	for (epp = &pat[0]; epp[1] != 0; ++epp)
		;
	clp = curwp->w_dotp;
	cbo = curwp->w_doto;
	for (;;) {
		if (cbo == 0) {
			clp = lback(clp);
			if (clp == curbp->b_linep)
				return (FALSE);
			cbo = llength(clp)+1;
		}
		if (--cbo == llength(clp))
			c = '\n';
		else
			c = lgetc(clp,cbo);
		if (eq(c, *epp) != FALSE) {
			tlp = clp;
			tbo = cbo;
			pp  = epp;
			while (pp != &pat[0]) {
				if (tbo == 0) {
					tlp = lback(tlp);
					if (tlp == curbp->b_linep)
						goto fail;
					tbo = llength(tlp)+1;
				}
				if (--tbo == llength(tlp))
					c = '\n';
				else
					c = lgetc(tlp,tbo);
				if (eq(c, *--pp) == FALSE)
					goto fail;
			}
			curwp->w_dotp  = tlp;
			curwp->w_doto  = tbo;
			curwp->w_flag |= WFMOVE;
			return (TRUE);
		}
	fail:	;
	}
}

/*
 * Compare two characters.
 * The "bc" comes from the buffer.
 * It has its case folded out. The
 * "pc" is from the pattern.
 */
eq(bc, pc)
{
	register int	ibc;
	register int	ipc;

	ibc = bc & 0xFF;
	ipc = pc & 0xFF;
	if (ISLOWER(ibc) != FALSE)
		ibc = TOUPPER(ibc);
	if (ISLOWER(ipc) != FALSE)
		ipc = TOUPPER(ipc);
	if (ibc == ipc)
		return (TRUE);
	return (FALSE);
}

/*
 * Read a pattern.
 * Stash it in the external variable "pat". The "pat" is
 * not updated if the user types in an empty line. If the user typed
 * an empty line, and there is no old pattern, it is an error.
 * Display the old pattern, in the style of Jeff Lomicka. There is
 * some do-it-yourself control expansion.
 */
readpattern(prompt)
char	*prompt;
{
	register int	s;
	char		tpat[NPAT];

	s = ereply("%s [%s]: ", tpat, NPAT, prompt, pat);
	if (s == TRUE)				/* Specified		*/
		strcpy(pat, tpat);
	else if (s==FALSE && pat[0]!=0)		/* CR, but old one	*/
		s = TRUE;
	return (s);
}
