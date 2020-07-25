/*
 * Name:	MicroEMACS
 *		VAX/VMS spawn a DCL subprocess.
 * Version:	29
 * Last edit:	05-Feb-86
 * By:		rex::conroy
 *		decvax!decwrl!dec-rhea!dec-rex!conroy
 */
#include	"def.h"

#include	<ssdef.h>
#include	<stsdef.h>
#include	<descrip.h>
#include	<iodef.h>

#define	EFN	0				/* Event flag.		*/

extern	int	oldmode[3];			/* In "termio.c".	*/
extern	int	newmode[3];
extern	short	iochan;

/*
 * Create a subjob with a copy
 * of the command intrepreter in it. When the
 * command interpreter exits, mark the screen as
 * garbage so that you do a full repaint. Bound
 * to "C-C" and called from "C-Z". The message at
 * the start in VMS puts out a newline. Under
 * some (unknown) condition, you don't get one
 * free when DCL starts up.
 */
spawncli(f, n, k)
{
	ttcolor(CTEXT);				/* Normal color.	*/
	ttwindow(0, nrow-1);			/* Full screen scroll.	*/
	ttmove(nrow-1, 0);			/* Last line.		*/
	eputs("[Starting DCL]");
	ttputc('\r');
	ttputc('\n');
	ttflush();
	sgarbf = TRUE;
	return (sys(NULL));			/* NULL => DCL.		*/
}

/*
 * Run a command. The "cmd" is a pointer
 * to a command string, or NULL if you want to run
 * a copy of DCL in the subjob (this is how the standard
 * routine LIB$SPAWN works. You have to do wierd stuff
 * with the terminal on the way in and the way out,
 * because DCL does not want the channel to be
 * in raw mode.
 */
sys(cmd)
register char	*cmd;
{
	struct	dsc$descriptor	cdsc;
	struct	dsc$descriptor	*cdscp;
	long	status;
	long	substatus;
	long	iosb[2];

	status = SYS$QIOW(EFN, iochan, IO$_SETMODE, iosb, 0, 0,
			  oldmode, sizeof(oldmode), 0, 0, 0, 0);
	if (status!=SS$_NORMAL || (iosb[0]&0xFFFF)!=SS$_NORMAL)
		return (FALSE);
	cdscp = NULL;				/* Assume DCL.		*/
	if (cmd != NULL) {			/* Build descriptor.	*/
		cdsc.dsc$a_pointer = cmd;
		cdsc.dsc$w_length  = strlen(cmd);
		cdsc.dsc$b_dtype   = DSC$K_DTYPE_T;
		cdsc.dsc$b_class   = DSC$K_CLASS_S;
		cdscp = &cdsc;
	}
	status = LIB$SPAWN(cdscp, 0, 0, 0, 0, 0, &substatus, 0, 0, 0);
	if (status != SS$_NORMAL)
		substatus = status;
	status = SYS$QIOW(EFN, iochan, IO$_SETMODE, iosb, 0, 0,
			  newmode, sizeof(newmode), 0, 0, 0, 0);
	if (status!=SS$_NORMAL || (iosb[0]&0xFFFF)!=SS$_NORMAL)
		return (FALSE);
	if ((substatus&STS$M_SUCCESS) == 0)	/* Command failed.	*/
		return (FALSE);
	return (TRUE);
}
