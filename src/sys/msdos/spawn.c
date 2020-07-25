/*
 * Name:	MicroEMACS
 *		MS-DOS spawn command.com
 * Version:	29
 * Last edit:	05-Feb-86
 * By:		rex::conroy
 *		decvax!decwrl!dec-rhea!dec-rex!conroy
 */
#include	"def.h"

#include	<dos.h>

#if	!LARGE
char	*cspec	= NULL;				/* Command string.	*/
#endif

/*
 * Create a subjob with a copy
 * of the command intrepreter in it. When the
 * command interpreter exits, mark the screen as
 * garbage so that you do a full repaint. Bound
 * to "C-C" and called from "C-Z".
 */
spawncli(f, n, k)
{
#if	LARGE
	eprintf("Not in large model MS-DOS");
	return (FALSE);
#else
	ttcolor(CTEXT);				/* Normal color.	*/
	ttwindow(0, nrow-1);			/* Full screen scroll.	*/
	ttmove(nrow-1, 0);			/* Last line.		*/
	ttflush();
	if (cspec == NULL) {			/* Try to find it.	*/
		cspec = getenv("COMSPEC");
		if (cspec == NULL)
			cspec = "A:COMMAND.COM";
	}
	sys(cspec, "");				/* Run CLI.		*/
	sgarbf = TRUE;
	return(TRUE);
#endif
}

#if	!LARGE
/*
 * This routine, once again
 * by Bob McNamara, is a C translation
 * of the "system" routine in the MWC-86 run
 * time library. It differs from the "system" routine
 * in that it does not unconditionally append the
 * string ".exe" to the end of the command name.
 * We needed to do this because we want to be
 * able to spawn off "command.com". We really do
 * not understand what it does, but if you don't
 * do it exactly "malloc" starts doing very
 * very strange things.
 */
sys(cmd, tail)
char	*cmd;
char	*tail;
{
	register unsigned n;
	extern   char	  *__end;

	n = __end + 15;
	n >>= 4;
	n = ((n + dsreg() + 16) & 0xFFF0) + 16;
	return(execall(cmd, tail, n));
}
#endif
