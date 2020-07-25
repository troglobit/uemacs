/*
 * Name:	MicroEMACS
 *		CP/M-86 spawn a sub-CLI (ha-ha).
 * Version:	29
 * Last edit:	05-Feb-86
 * By:		rex::conroy
 *		decvax!decwrl!dec-rhea!dec-rex!conroy
 */
#include	"def.h"

/*
 * Create a subjob with a copy
 * of the command intrepreter in it. When the
 * command interpreter exits, mark the screen as
 * garbage so that you do a full repaint. Bound
 * to "C-C" and called from "C-Z".
 */
spawncli(f, n, k)
{
	eprintf("Not in CP/M-86");
	return (FALSE);
}
