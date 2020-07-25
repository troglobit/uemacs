/*
 * Name:	MicroEMACS
 *		Heath H19 keyboard
 * Version:	29
 * Last edit:	05-Feb-86
 * By:		rex::conroy
 *		decvax!decwrl!dec-rhea!dec-rex!conroy
 */
#include	"def.h"

/*
 * Empty key name table.
 */
char	*keystrings[] = {
	NULL,		NULL,		NULL,		NULL,
	NULL,		NULL,		NULL,		NULL,
	NULL,		NULL,		NULL,		NULL,
	NULL,		NULL,		NULL,		NULL,
	NULL,		NULL,		NULL,		NULL,
	NULL,		NULL,		NULL,		NULL,
	NULL,		NULL,		NULL,		NULL,
	NULL,		NULL,		NULL,		NULL
};

/*
 * Get keyboard character, and interpret
 * any special keys on the keyboard. This is really
 * easy, because there arn't any special keys.
 */
getkbd()
{
	return (ttgetc());
}

/*
 * No special keys.
 */
ttykeymapinit()
{
}
