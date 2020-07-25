/*
 * Name:	MicroEMACS
 *		Spawn CLI; stop if C shell.
 * Version:	29
 * Last edit:	10-Feb-86
 * By:		rex::conroy
 *		decvax!decwrl!dec-rhea!dec-rex!conroy
 *
 * Spawn. New version, which
 * interracts with the job control stuff
 * in the 4.X BSD C shell.
 */
#include	"def.h"

#include	<sgtty.h>
#include	<signal.h>

char	*shellp	= NULL;			/* Saved "SHELL" name.		*/

extern	struct	sgttyb	oldtty;		/* There really should be a	*/
extern	struct	sgttyb	newtty;		/* nicer way of doing this, so	*/
extern	struct	sgttyb	oldtchars;	/* spawn does not need to know	*/
extern	struct	sgttyb	newtchars;	/* about the insides of the	*/
extern	struct	sgttyb	oldltchars;	/* terminal I/O code.		*/
extern	struct	sgttyb	newltchars;

extern	char	*getenv();

/*
 * This code does a one of 2 different
 * things, depending on what version of the shell
 * you are using. If you are using the C shell, which
 * implies that you are using job control, then MicroEMACS
 * moves the cursor to a nice place and sends itself a
 * stop signal. If you are using the Bourne shell it runs
 * a subshell using fork/exec. Bound to "C-C", and used
 * as a subcommand by "C-Z".
 */
spawncli(f, n, k)
{
	register int	pid;
	register int	wpid;
	register int	(*oqsig)();
	register int	(*oisig)();
	int		status;

	if (shellp == NULL) {
		shellp = getenv("SHELL");
		if (shellp == NULL)
			shellp = getenv("shell");
		if (shellp == NULL)
			shellp = "/bin/sh";	/* Safer.		*/
	}
	ttcolor(CTEXT);
	ttnowindow();
	if (strcmp(shellp, "/bin/csh") == 0) {
		if (epresf != FALSE) {
			ttmove(nrow-1, 0);
			tteeol();
			epresf = FALSE;
		}				/* Csh types a "\n"	*/
		ttmove(nrow-2, 0);		/* before "Stopped".	*/
	} else {
		ttmove(nrow-1, 0);
		if (epresf != FALSE) {
			tteeol();
			epresf = FALSE;
		}
	}
	ttflush();
	if (ioctl(0, TIOCSLTC, &oldltchars) < 0
	||  ioctl(0, TIOCSETC, &oldtchars)  < 0
	||  ioctl(0, TIOCSETP, &oldtty)     < 0) {
		eprintf("IOCTL #1 to terminal failed");
		return (FALSE);
	}
	if (strcmp(shellp, "/bin/csh") == 0)	/* C shell.		*/
		kill(0, SIGTSTP);
	else {					/* Bourne shell.	*/
		oqsig = signal(SIGQUIT, SIG_IGN);
		oisig = signal(SIGINT,  SIG_IGN);
		if ((pid=fork()) < 0) {
			signal(SIGQUIT, oqsig);
			signal(SIGINT,  oisig);
			eprintf("Failed to create process");
			return (FALSE);
		}
		if (pid == 0) {
			execl(shellp, "sh", "-i", NULL);
			_exit(0);		/* Should do better!	*/
		}
		while ((wpid=wait(&status))>=0 && wpid!=pid)
			;
		signal(SIGQUIT, oqsig);
		signal(SIGINT,  oisig);
	}
	sgarbf = TRUE;				/* Force repaint.	*/
	if (ioctl(0, TIOCSETP, &newtty)     < 0
	||  ioctl(0, TIOCSETC, &newtchars)  < 0
	||  ioctl(0, TIOCSLTC, &newltchars) < 0) {
		eprintf("IOCTL #2 to terminal failed");
		return (FALSE);
	}
	return (TRUE);
}
