/*
 * Name:	MicroEMACS
 *		CP/M-86 terminal I/O.
 * Version:	29
 * Last edit:	05-Feb-86
 * By:		rex::conroy
 *		decvax!decwrl!dec-rhea!dec-rex!conroy
 */
#include	"def.h"

#include	<bdos.h>

int	nrow;				/* Terminal size, rows.		*/
int	ncol;				/* Terminal size, columns.	*/

/*
 * Set up terminal.
 * Almost no operation in CP/M-86.
 */
ttopen()
{
	nrow = NROW;
	ncol = NCOL;
}

/*
 * No operation in CP/M-86.
 */
ttclose()
{
}

/*
 * Write character.
 */
ttputc(c)
{
	bios(BCONOUT, c, 0);
}

/*
 * No operation on CP/M-86.
 */
ttflush()
{
}

/*
 * Read character.
 */
ttgetc()
{
	return (biosb(BCONIN, 0, 0));
}
