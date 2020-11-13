/*
 * Name:	MicroEMACS
 *		Mainline, macro commands.
 * Version:	29
 * Last edit:	05-Feb-86
 * By:		rex::conroy
 *		decvax!decwrl!dec-rhea!dec-rex!conroy
 */
#include	"def.h"

int	thisflag;			/* Flags, this command		*/
int	lastflag;			/* Flags, last command		*/
int	curgoal;			/* Goal column			*/
BUFFER	*curbp;				/* Current buffer		*/
WINDOW	*curwp;				/* Current window		*/
BUFFER	*bheadp;			/* BUFFER listhead		*/
WINDOW	*wheadp;			/* WINDOW listhead		*/
BUFFER	*blistp;			/* Buffer list BUFFER		*/
short	kbdm[NKBDM] = { KCTLX | ')' };	/* Macro			*/
short	*kbdmip;			/* Input  for above		*/
short	*kbdmop;			/* Output for above		*/
char	pat[NPAT];			/* Pattern			*/
SYMBOL	*symbol[NSHASH];		/* Symbol table listhead.	*/
SYMBOL	*binding[NKEYS];		/* Key bindings.		*/

main(argc, argv)
char	*argv[];
{
	register int	c;
	register int	f;
	register int	n;
	register int	mflag;
	char		bname[NBUFN];

	strcpy(bname, "main");			/* Get buffer name.	*/
	if (argc > 1)
		makename(bname, argv[1]);
	vtinit();				/* Virtual terminal.	*/
	edinit(bname);				/* Buffers, windows.	*/
	keymapinit();				/* Symbols, bindings.	*/
	if (argc > 1) {
		update();
		readin(argv[1]);
	}
	lastflag = 0;				/* Fake last flags.	*/
loop:
	update();				/* Fix up the screen.	*/
	c = getkey();
	if (epresf != FALSE) {
		eerase();
		update();
	}
	f = FALSE;
	n = 1;
	if (c == (KCTRL|'U')) {			/* ^U, start argument.	*/
		f = TRUE;
		n = 4;
		while ((c=getkey()) == (KCTRL|'U'))
			n *= 4;
		if ((c>='0' && c<='9') || c=='-') {
			if (c == '-') {
				n = 0;
				mflag = TRUE;
			} else {
				n = c - '0';
				mflag = FALSE;
			}
			while ((c=getkey())>='0' && c<='9')
				n = 10*n + c - '0';
			if (mflag != FALSE)
				n = -n;
		}
	}
	if (kbdmip != NULL) {			/* Save macro strokes.	*/
		if (c!=(KCTLX|')') && kbdmip>&kbdm[NKBDM-6]) {
			ctrlg(FALSE, 0, KRANDOM);
			goto loop;
		}
		if (f != FALSE) {
			*kbdmip++ = (KCTRL|'U');
			*kbdmip++ = n;
		}
		*kbdmip++ = c;
	}
	execute(c, f, n);			/* Do it.		*/
	goto loop;
}

/*
 * Command execution. Look up the binding in the the
 * binding array, and do what it says. Return a very bad status
 * if there is no binding, or if the symbol has a type that
 * is not usable (there is no way to get this into a symbol table
 * entry now). Also fiddle with the flags.
 */
execute(c, f, n)
{
	register SYMBOL	*sp;
	register int	status;

	if ((sp=binding[c]) != NULL) {
		thisflag = 0;
		status = (*sp->s_funcp)(f, n, c);
		lastflag = thisflag;
		return (status);
	}
	lastflag = 0;
	return (ABORT);
}

/*
 * Initialize all of the buffers
 * and windows. The buffer name is passed down as
 * an argument, because the main routine may have been
 * told to read in a file by default, and we want the
 * buffer name to be right.
 */
edinit(bname)
char	bname[];
{
	register BUFFER	*bp;
	register WINDOW	*wp;

	bp = bfind(bname, TRUE);		/* Text buffer.		*/
	blistp = bcreate("");			/* Special list buffer.	*/
	wp = (WINDOW *) malloc(sizeof(WINDOW));	/* Initial window.	*/
	if (bp==NULL || wp==NULL || blistp==NULL)
		abort();
	curbp  = bp;				/* Current ones.	*/
	wheadp = wp;
	curwp  = wp;
	wp->w_wndp  = NULL;			/* Initialize window.	*/
	wp->w_bufp  = bp;
	bp->b_nwnd  = 1;			/* Displayed.		*/
	wp->w_linep = bp->b_linep;
	wp->w_dotp  = bp->b_linep;
	wp->w_doto  = 0;
	wp->w_markp = NULL;
	wp->w_marko = 0;
	wp->w_toprow = 0;
	wp->w_ntrows = nrow-2;			/* 2 = mode, echo.	*/
	wp->w_force = 0;
	wp->w_flag  = WFMODE|WFHARD;		/* Full.		*/
}
	
/*
 * Fancy quit command, as implemented
 * by Jeff. If the current buffer has changed
 * do a write current buffer. Otherwise run a command
 * interpreter in a subjob. Two of these will get you
 * out. Bound to "C-Z".
 */
jeffexit(f, n, k)
{
	if ((curbp->b_flag&BFCHG) != 0)		/* Changed.		*/
		return (filesave(f, n, KRANDOM));
	return (spawncli(f, n, KRANDOM));	/* Suspend.		*/
}

/*
 * Quit command. If an argument, always
 * quit. Otherwise confirm if a buffer has been
 * changed and not written out. Normally bound
 * to "C-X C-C".
 */
quit(f, n, k)
{
	register int	s;

	if (f != FALSE				/* Argument forces it.	*/
	|| anycb() == FALSE			/* All buffers clean.	*/
	|| (s=eyesno("Quit")) == TRUE) {	/* User says it's OK.	*/
		vttidy();
		exit(GOOD);
	}
	return (s);
}

/*
 * Begin a keyboard macro.
 * Error if not at the top level
 * in keyboard processing. Set up
 * variables and return.
 */
ctlxlp(f, n, k)
{
	if (kbdmip!=NULL || kbdmop!=NULL) {
		eprintf("Not now");
		return (FALSE);
	}
	eprintf("[Start macro]");
	kbdmip = &kbdm[0];
	return (TRUE);
}

/*
 * End keyboard macro. Check for
 * the same limit conditions as the
 * above routine. Set up the variables
 * and return to the caller.
 */
ctlxrp(f, n, k)
{
	if (kbdmip == NULL) {
		eprintf("Not now");
		return (FALSE);
	}
	eprintf("[End macro]");
	kbdmip = NULL;
	return (TRUE);
}

/*
 * Execute a macro.
 * The command argument is the
 * number of times to loop. Quit as
 * soon as a command gets an error.
 * Return TRUE if all ok, else
 * FALSE.
 */
ctlxe(f, n, k)
{
	register int	c;
	register int	af;
	register int	an;
	register int	s;

	if (kbdmip!=NULL || kbdmop!=NULL) {
		eprintf("Not now");
		return (FALSE);
	}
	if (n <= 0) 
		return (TRUE);
	do {
		kbdmop = &kbdm[0];
		do {
			af = FALSE;
			an = 1;
			if ((c = *kbdmop++) == (KCTRL|'U')) {
				af = TRUE;
				an = *kbdmop++;
				c  = *kbdmop++;
			}
			s = TRUE;
		} while (c!=(KCTLX|')') && (s=execute(c, af, an))==TRUE);
		kbdmop = NULL;
	} while (s==TRUE && --n);
	return (s);
}

/*
 * Abort.
 * Beep the beeper.
 * Kill off any keyboard macro,
 * etc., that is in progress.
 * Sometimes called as a routine,
 * to do general aborting of
 * stuff.
 */
ctrlg(f, n, k)
{
	ttbeep();
	if (kbdmip != NULL) {
		kbdm[0] = (KCTLX|')');
		kbdmip  = NULL;
	}
	return (ABORT);
}

/*
 * Display the version. All this does
 * is copy the text in the external "version" array into
 * the message system, and call the message reading code.
 * Don't call display if there is an argument.
 */
showversion(f, n, k)
{
	register char	**cpp;
	register char	*cp;

	cpp = &version[0];
	while ((cp = *cpp++) != NULL) {
		if (writemsg(cp) == FALSE)
			return (FALSE);
	}
	if (f != FALSE)				/* No display if arg.	*/
		return (TRUE);
	return (readmsg());
}
