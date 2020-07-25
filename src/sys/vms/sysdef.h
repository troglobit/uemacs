/*
 * Name:	MicroEMACS
 *		VAX/VMS system header file.
 * Version:	29
 * Last edit:	05-Feb-86
 * By:		rex::conroy
 *		decvax!decwrl!dec-rhea!dec-rex!conroy
 */
#include	<ssdef.h>

#define	PCC	0			/* "[]" works.			*/
#define	KBLOCK	8192			/* Kill grow.			*/
#define	GOOD	(SS$_NORMAL)		/* Good exit status.		*/

/*
 * Macros used by the buffer name making code.
 * Start at the end of the file name, scan to the left
 * until BDC1 (or BDC2, if defined) is reached. The buffer
 * name starts just to the right of that location, and
 * stops at end of string (or at the next BDC3 character,
 * if defined). BDC2 and BDC3 are mainly for VMS.
 */
#define	BDC1	':'			/* Buffer names.		*/
#define	BDC2	']'
#define	BDC3	';'
