/*
 * Name:	MicroEMACS
 *		Atari 520ST keyboard.
 * Version:	30
 * Last edit:	22-Feb-86
 * By:		rex::conroy
 *		decvax!decwrl!dec-rhea!dec-rex!conroy
 */
#include	"def.h"
#include	<osbind.h>

/*
 * Key names.
 */
char	*keystrings[] = {
	NULL,		"F1",		"F2",		"F3",
	"F4",		"F5",		"F6",		"F7",
	"F8",		"F9",		"F10",		"Help",
	"Undo",		"Insert",	"Up",		"Clr/Home",
	"Left",		"Down",		"Right",	NULL,
	NULL,		NULL,		NULL,		NULL,
	NULL,		NULL,		NULL,		NULL,
	NULL,		NULL,		NULL,		NULL
};

/*
 * This function is used to read the
 * keyboard for the first time. A call to the system
 * is made directly, to see the 32 bit key code. If the
 * scan code says this is a function key, remap the
 * codes. I might want to make the "Alt" key work like
 * a "Meta" key, by looking at scan code.
 */
getkbd()
{
	register long	rawkey;
	register int	k;

	rawkey = Crawcin();
	k = (rawkey>>16) & 0xFF;		/* Scan code.		*/
	if (k == 0x3B)
		return (KF1);
	if (k == 0x3C)
		return (KF2);
	if (k == 0x3D)
		return (KF3);
	if (k == 0x3E)
		return (KF4);
	if (k == 0x3F)
		return (KF5);
	if (k == 0x40)
		return (KF6);
	if (k == 0x41)
		return (KF7);
	if (k == 0x42)
		return (KF8);
	if (k == 0x43)
		return (KF9);
	if (k == 0x44)
		return (KF10);
	if (k == 0x62)
		return (KHELP);
	if (k == 0x61)
		return (KUNDO);
	if (k == 0x52)
		return (KINSERT);
	if (k == 0x48)
		return (KUP);
	if (k == 0x47)
		return (KCLEAR);
	if (k == 0x4B)
		return (KLEFT);
	if (k == 0x50)
		return (KDOWN);
	if (k == 0x4D)
		return (KRIGHT);
	return ((int)(rawkey&0x7F));
}

/*
 * Establish default keypad
 * bindings. The "Undo" key is bound to the
 * "execute-macro"; this is where I bind "Do" on
 * an LK201, and it is very handy.
 * All of the Fn keys are bindable, but there
 * are no default bindings.
 */
ttykeymapinit()
{
	keydup(KHELP,	"help");
	keydup(KUNDO,	"execute-macro");
	keydup(KINSERT,	"yank");
	keydup(KUP,	"back-line");
	keydup(KCLEAR,	"kill-region");
	keydup(KLEFT,	"back-char");
	keydup(KDOWN,	"forw-line");
	keydup(KRIGHT,	"forw-char");
}
