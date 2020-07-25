/*
 * Name:	MicroEMACS
 *		Atari 520ST terminal I/O.
 * Version:	30
 * Last edit:	05-Feb-86
 * By:		rex::conroy
 *		decvax!decwrl!dec-rhea!dec-rex!conroy
 */
#include	"def.h"
#include	<osbind.h>

int	nrow;				/* Terminal size, rows.		*/
int	ncol;				/* Terminal size, columns.	*/

/*
 * Open. Determine the size of
 * the display by calling the assembly
 * language "getnrow" and "getncol"
 * routines.
 */
ttopen()
{
	nrow = getnrow();
	if (nrow > NROW)
		nrow = NROW;
	ncol = getncol();
	if (ncol > NCOL)
		ncol = NCOL;
}
/*
 * No-op.
 */
ttclose()
{
}

/*
 * Put character.
 */
ttputc(c)
{
	Crawio(c & 0x7F);
}

/*
 * No-op.
 */
ttflush()
{
}

/*
 * Get characters.
 */
ttgetc()
{
	return (Crawcin() & 0x7F);
}
