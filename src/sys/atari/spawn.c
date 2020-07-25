/*
 * Name:	MicroEMACS
 *		Atari 520ST CLI spawn.
 * Version:	30
 * Last edit:	22-Feb-86
 * By:		rex::conroy
 *		decvax!decwrl!dec-rhea!dec-rex!conroy
 */
#include	"def.h"

/*
 * Spawn CLI. Since MicroEMACS wants to hold
 * a lot of memory, this may never be made to work right
 * in GEMDOS. On the other hand, it sure would be a
 * nice thing to have.
 */
spawncli(f, n, k)
{
	eprintf("Not in GEMDOS");
	return (FALSE);
}
