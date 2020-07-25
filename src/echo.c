/*
 * Name:	MicroEMACS
 *		Echo line reading and writing.
 * Version:	29
 * Last edit:	14-Feb-86
 * By:		rex::conroy
 *		decvax!decwrl!dec-rhea!dec-rex!conroy
 *
 * Common routines for reading
 * and writing characters in the echo line area
 * of the display screen. Used by the entire
 * known universe.
 */
#include	"def.h"

int	epresf	= FALSE;		/* Stuff in echo line flag.	*/
int	nmsg	= 0;			/* Size of occupied msg. area.	*/
int	curmsgf	= FALSE;		/* Current alert state.		*/
int	newmsgf	= FALSE;		/* New alert state.		*/

char	msg[NMSG];			/* Random message storage.	*/

/*
 * Send a string to the message system.
 * Add a free newline to the end of the message string.
 * Return TRUE if it fits, and FALSE if it does not.
 * Perhaps the message buffer should know how to get
 * larger, just like the kill buffer?
 */
writemsg(sp)
register char	*sp;
{
	register int	c;

	if (nmsg+strlen(sp)+1 > NMSG)		/* "+1" for the "\n".	*/
		return (FALSE);
	while ((c = *sp++) != '\0')
		msg[nmsg++] = c;
	msg[nmsg++] = '\n';
	newmsgf = TRUE;				/* Update mode line.	*/
	return (TRUE);
}

/*
 * Read messages. The message lines are
 * displayed, one line at a time, in the message line.
 * A special sub-mode is entered, in which the keys have
 * the following meanings:
 *	^P	Go backward 1 line.
 *	BS	Go backward 1 line.
 *	^N	Go forward 1 line. Quit if at the end.
 *	SP	Go forward 1 line. Quit if at the end.
 *	CR	Go forward 1 line. Quit if at the end.
 *	^G	Abort, leave old text.
 *	^C	Quit, delete anything already read.
 * Return TRUE if you left this mode in a reasonable
 * way (not ^G), and ABORT if you quit the mode with a
 * ^G.
 */
readmsg()
{
	register int	c;
	register int	i;
	register int	j;

	if (nmsg == 0)				/* Duck out if none.	*/
		return (TRUE);
	newmsgf = FALSE;			/* Kill alert, and do	*/
	update();				/* a redisplay.		*/
	ttcolor(CTEXT);
	i = 0;
	while (i < nmsg) {
		ttmove(nrow-1, 0);		/* Display 1 line.	*/
		while (i<nmsg && (c=msg[i++])!='\n')
			eputc(c);
		tteeol();
		ttmove(nrow-1, 0);		/* Looks nice.		*/
		ttflush();
		for (;;) {			/* Editing loop.	*/
			c = ttgetc();
			switch (c) {
			case 0x0E:		/* ^N			*/
			case 0x20:		/* SP			*/
			case 0x0D:		/* CR			*/
				break;

			case 0x10:		/* ^P			*/
			case 0x08:		/* BS			*/
				do {
					--i;
				} while (i!=0 && msg[i-1]!='\n');
				if (i != 0) {
					do {	/* Back up 1 line.	*/
						--i;
					} while (i!=0 && msg[i-1]!='\n');
				}
				break;

			case 0x03:		/* ^C			*/
				j = 0;		/* Eat what we read.	*/
				while (i < nmsg)
					msg[j++] = msg[i++];
				nmsg = j;
				eerase();
				return (TRUE);

			case 0x07:		/* ^G			*/
				ttbeep();
				eerase();
				return (ABORT);

			default:		/* Loop on the rest.	*/
				continue;
			}
			break;
		}				
	}	
	nmsg = 0;				/* Flow off the end.	*/
	eerase();
	return (TRUE);
}

/*
 * Erase the echo line.
 */
eerase()
{
	ttcolor(CTEXT);
	ttmove(nrow-1, 0);
	tteeol();
	ttflush();
	epresf = FALSE;
}

/*
 * Ask "yes" or "no" question.
 * Return ABORT if the user answers the question
 * with the abort ("^G") character. Return FALSE
 * for "no" and TRUE for "yes". No formatting
 * services are available.
 */
eyesno(sp)
char	*sp;
{
	register int	s;
	char		buf[64];

	for (;;) {
		s = ereply("%s [y/n]? ", buf, sizeof(buf), sp);
		if (s == ABORT)
			return (ABORT);
		if (s != FALSE) {
			if (buf[0]=='y' || buf[0]=='Y')
				return (TRUE);
			if (buf[0]=='n' || buf[0]=='N')
				return (FALSE);
		}
	}
}

/*
 * Write out a prompt, and read back a
 * reply. The prompt is now written out with full "eprintf"
 * formatting, although the arguments are in a rather strange
 * place. This is always a new message, there is no auto
 * completion, and the return is echoed as such.
 */
/* VARARGS3 */
ereply(fp, buf, nbuf, arg)
char	*fp;
char	*buf;
{
	return (eread(fp, buf, nbuf, EFNEW|EFCR, (char *)&arg));
}

/*
 * This is the general "read input from the
 * echo line" routine. The basic idea is that the prompt
 * string "prompt" is written to the echo line, and a one
 * line reply is read back into the supplied "buf" (with
 * maximum length "len"). The "flag" contains EFNEW (a
 * new prompt), an EFAUTO (autocomplete), or EFCR (echo
 * the carriage return as CR).
 */
eread(fp, buf, nbuf, flag, ap)
char	*fp;
char	*buf;
char	*ap;
{
	register int	cpos;
	register SYMBOL	*sp1;
	register SYMBOL	*sp2;
	register int	i;
	register int	c;
	register int	h;
	register int	nhits;
	register int	nxtra;
	register int	bxtra;

	cpos = 0;
	if (kbdmop != NULL) {			/* In a macro.		*/
		while ((c = *kbdmop++) != '\0')
			buf[cpos++] = c;
		buf[cpos] = '\0';
		goto done;
	}
	if ((flag&EFNEW)!=0 || ttrow!=nrow-1) {
		ttcolor(CTEXT);
		ttmove(nrow-1, 0);
		epresf = TRUE;
	} else
		eputc(' ');
	eformat(fp, ap);
	tteeol();
	ttflush();
	for (;;) {
		c = ttgetc();
		if (c==' ' && (flag&EFAUTO)!=0) {
			nhits = 0;
			nxtra = HUGE;
			for (h=0; h<NSHASH; ++h) {
				sp1 = symbol[h];
				while (sp1 != NULL) {
					for (i=0; i<cpos; ++i) {
						if (buf[i] != sp1->s_name[i])
							break;
					}
					if (i == cpos) {
						if (nhits == 0)
							sp2 = sp1;
						++nhits;
						bxtra = getxtra(sp1, sp2, cpos);
						if (bxtra < nxtra)
							nxtra = bxtra;
					}
					sp1 = sp1->s_symp;
				}
			}
			if (nhits == 0)		/* No completion.	*/
				continue;
			for (i=0; i<nxtra && cpos<nbuf-1; ++i) {
				c = sp2->s_name[cpos];
				buf[cpos++] = c;
				eputc(c);
			}
			ttflush();
			if (nhits != 1)		/* Fake a CR if there	*/
				continue;	/* is 1 choice.		*/
			c = 0x0D;
		}
		switch (c) {
		case 0x0D:			/* Return, done.	*/
			buf[cpos] = '\0';
			if (kbdmip != NULL) {
				if (kbdmip+cpos+1 > &kbdm[NKBDM-3]) {
					(void) ctrlg(FALSE, 0, KRANDOM);
					ttflush();
					return (ABORT);
				}
				for (i=0; i<cpos; ++i)
					*kbdmip++ = buf[i];
				*kbdmip++ = '\0';
			}
			if ((flag&EFCR) != 0) {
				ttputc(0x0D);
				ttflush();
			}
			goto done;

		case 0x07:			/* Bell, abort.		*/
			eputc(0x07);
			(void) ctrlg(FALSE, 0, KRANDOM);
			ttflush();
			return (ABORT);

		case 0x7F:			/* Rubout, erase.	*/
		case 0x08:			/* Backspace, erase.	*/
			if (cpos != 0) {
				ttputc('\b');
				ttputc(' ');
				ttputc('\b');
				--ttcol;
				if (ISCTRL(buf[--cpos]) != FALSE) {
					ttputc('\b');
					ttputc(' ');
					ttputc('\b');
					--ttcol;
				}
				ttflush();
			}
			break;

		case 0x15:			/* C-U, kill line.	*/
			while (cpos != 0) {
				ttputc('\b');
				ttputc(' ');
				ttputc('\b');
				--ttcol;
				if (ISCTRL(buf[--cpos]) != FALSE) {
					ttputc('\b');
					ttputc(' ');
					ttputc('\b');
					--ttcol;
				}
			}
			ttflush();
			break;

		default:			/* All the rest.	*/
			if (cpos < nbuf-1) {
				buf[cpos++] = c;
				eputc(c);
				ttflush();
			}
		}
	}
done:
	if (buf[0] == '\0')
		return (FALSE);
	return (TRUE);
}

/*
 * The "sp1" and "sp2" point to extended command
 * symbol table entries. The "cpos" is a horizontal position
 * in the name. Return the longest block of characters that can
 * be autocompleted at this point. Sometimes the two symbols
 * are the same, but this is normal.
 */
getxtra(sp1, sp2, cpos)
register SYMBOL	*sp1;
register SYMBOL	*sp2;
{
	register int	i;

	i = cpos;
	for (;;) {
		if (sp1->s_name[i] != sp2->s_name[i])
			break;
		if (sp1->s_name[i] == '\0')
			break;
		++i;
	}
	return (i - cpos);
}

/*
 * Special "printf" for the echo line.
 * Each call to "eprintf" starts a new line in the
 * echo area, and ends with an erase to end of the
 * echo line. The formatting is done by a call
 * to the standard formatting routine.
 */
/* VARARGS1 */
eprintf(fp, arg)
char	*fp;
{
	ttcolor(CTEXT);
	ttmove(nrow-1, 0);
	eformat(fp, (char *)&arg);
	tteeol();
	ttflush();
	epresf = TRUE;
}

/*
 * Printf style formatting. This is
 * called by both "eprintf" and "ereply", to provide
 * formatting services to their clients. The move to the
 * start of the echo line, and the erase to the end of
 * the echo line, is done by the caller.
 */
eformat(fp, ap)
register char	*fp;
register char	*ap;
{
	register int	c;

	while ((c = *fp++) != '\0') {
		if (c != '%')
			eputc(c);
		else {
			c = *fp++;
			switch (c) {
			case 'd':
				eputi(*(int *)ap, 10);
				ap += sizeof(int);
				break;

			case 'o':
				eputi(*(int *)ap,  8);
				ap += sizeof(int);
				break;

			case 's':
				eputs(*(char **)ap);
				ap += sizeof(char *);
				break;

			default:
				eputc(c);
			}
		}
	}
}

/*
 * Put integer, in radix "r".
 */
eputi(i, r)
register int	i;
register int	r;
{
	register int	q;

	if ((q=i/r) != 0)
		eputi(q, r);
	eputc(i%r+'0');
}

/*
 * Put string.
 */
eputs(s)
register char	*s;
{
	register int	c;

	while ((c = *s++) != '\0')
		eputc(c);
}

/*
 * Put character. Watch for
 * control characters, and for the line
 * getting too long.
 */
eputc(c)
register int	c;
{
	if (ttcol < ncol) {
		if (ISCTRL(c) != FALSE) {
			eputc('^');
			c ^= 0x40;
		}
		ttputc(c);
		++ttcol;
	}
}
