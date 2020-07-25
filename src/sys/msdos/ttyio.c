/*
 * Name:	MicroEMACS
 *		MS-DOS terminal I/O.
 * Version:	29
 * Last edit:	05-Feb-86
 * By:		rex::conroy
 *		decvax!decwrl!dec-rhea!dec-rex!conroy
 */
#include	"def.h"

#include	<dos.h>

int	nrow;				/* Terminal size, rows.		*/
int	ncol;				/* Terminal size, columns.	*/

/*
 * Initialization.
 * Almost no operation in MS-DOS.
 */
ttopen()
{
	nrow = NROW;
	ncol = NCOL;
}

/*
 * No operation in MS-DOS.
 */
ttclose()
{
}

/*
 * Write character.
 */
ttputc(c)
{
	dosb(CONDIO, c, 0);
}

/*
 * No operation in MS-DOS.
 */
ttflush()
{
}

/*
 * Read character.
 */
ttgetc()
{
	return (dosb(CONRAW,  0, 0));
}
