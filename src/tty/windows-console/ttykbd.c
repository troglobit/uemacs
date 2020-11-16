/*
 * ANSI input driver. Based on the original ANSI source code by Dave Conroy.
 *
 * Jörgen Sigvardsson <jorgen.sigvardsson@gmail.com>
 */
#include	"def.h"

#define	ESC	0x1B			/* Escape, arrows et al.	*/
#define	AGRAVE	0x60			/* LK201 kludge.		*/

/*
 * The special keys on an LK201 send back
 * escape sequences of the general form <ESC>[nnn~, where
 * nnn is a key code number. This table is indexed by the
 * nnn code to get the key code, which is used in the
 * binding table. The F4 key, and the F6 through F14 keys
 * have key codes. Also F17 through F20. Think of
 * help and do as special.
 */
short	lk201map[] = {
	KRANDOM,	KFIND,		KINSERT,	KREMOVE,
	KSELECT,	KPREV,		KNEXT,		KRANDOM,
	KRANDOM,	KRANDOM,	KRANDOM,	KRANDOM,
	KRANDOM,	KRANDOM,	KF4,		KRANDOM,
	KRANDOM,	KF6,		KF7,		KF8,
	KF9,		KF10,		KRANDOM,	KF11,
	KF12,		KF13,		KF14,		KRANDOM,
	KHELP,		KDO,		KRANDOM,	KF17,
	KF18,		KF19,		KF20
};

/*
 * Names for the keys with basic keycode
 * between KFIRST and KLAST (inclusive). This is used by
 * the key name routine in "kbd.c".
 */
char	*keystrings[] = {
	NULL,		"Up",		"Down",		"Left",
	"Right",	"Find",		"Insert",	"Remove",
	"Select",	"Previous",	"Next",		"F4",
	"F6",		"F7",		"F8",		"F9",
	"F10",		"F11",		"F12",		"F13",
	"F14",		"Help",		"Do",		"F17",
	"F18",		"F19",		"F20",		"PF1",
	"PF2",		"PF3",		"PF4",		NULL
};

/*
 * Read in a key, doing the low level mapping
 * of ASCII code to 11 bit code. This level deals with
 * mapping the special keys into their spots in the C1
 * control area. The C0 controls go right through, and
 * get remapped by "getkey".
 */
getkbd()
{
	register int	c;
	register int	n;
loop:
	c = ttgetc();
	if (c == AGRAVE)			/* On LK201, grave is	*/
		c = METACH;			/* also META.		*/
	if (c == ESC) {
		c = ttgetc();
		if (c == '[') {
			c = ttgetc();
			if (c == 'A')
				return (KUP);
			if (c == 'B')
				return (KDOWN);
			if (c == 'C')
				return (KRIGHT);
			if (c == 'D')
				return (KLEFT);
			if (c == 'F') /* Maps to the "End" key on PC/Windows */
				return KSELECT;
			if (c == 'H') /* Maps to the "Home" key on PC/Windows */
				return KFIND;
			if (c>='0' && c<='9') {	/* LK201 functions.	*/
				n = 0;
				do {
					n = 10*n + c - '0';
					c = ttgetc();
				} while (c>='0' && c<='9');
				if (c=='~' && n<=34) {
					c = lk201map[n];
					if (c != KRANDOM)
						return (c);
					goto loop;
				}
			}
			goto loop;
		}
		if (c == 'O') {	
			c = ttgetc();
			if (c == 'A')
				return (KUP);
			if (c == 'B')
				return (KDOWN);
			if (c == 'C')
				return (KRIGHT);
			if (c == 'D')
				return (KLEFT);
			if (c == 'P')
				return (KHELP); /* Maps to F1 on Windows/PC */
			if (c == 'Q')
				return (KPF2);
			if (c == 'R')
				return (KPF3);
			if (c == 'S')
				return (KPF4);
			goto loop;
		}
		if (ISLOWER(c) != FALSE)	/* Copy the standard	*/
			c = TOUPPER(c);		/* META code.		*/
		if (c>=0x00 && c<=0x1F)
			c = KCTRL | (c+'@');
		return (KMETA | c);
	}

	return (c);
}

/*
 * Terminal specific keymap initialization.
 * Attach the special keys to the appropriate built
 * in functions. Bind all of the assigned graphics in the
 * DEC supplimental character set to "ins-self".
 * As is the case of all the keymap routines, errors
 * are very fatal.
 */
ttykeymapinit()
{
	register SYMBOL	*sp;
	register int	i;

	keydup(KFIND,	"search-again");
	keydup(KHELP,	"help");
	keydup(KPF2,    "display-bindings");
	keydup(KINSERT, "yank");
	keydup(KREMOVE, "kill-region");
	keydup(KSELECT, "set-mark");
	keydup(KPREV,	"back-page");
	keydup(KNEXT,	"forw-page");
	keydup(KDO,	"execute-macro");
	keydup(KF17,	"back-window");
	keydup(KF18,	"forw-window");
	keydup(KF19,	"enlarge-window");
	keydup(KF20,	"shrink-window");
	keydup(KUP,	"back-line");
	keydup(KDOWN,	"forw-line");
	keydup(KRIGHT,	"forw-char");
	keydup(KLEFT,	"back-char");

	/*
	 * Bind all GR positions that correspond
	 * to assigned characters in the Digital multinational
	 * character set to "ins-self". These characters may
	 * be used just like any other character.
	 */

	if ((sp=symlookup("ins-self")) == NULL)
		abort();
	for (i=0xA0; i<0xFF; ++i) {
		if (i!=0xA4 && i!=0xA6 && i!=0xAC && i!=0xAD && i!=0xAE
		&&  i!=0xAF && i!=0xB4 && i!=0xB8 && i!=0xBE && i!=0xF0
		&&  i!=0xFE && i!=0xA0) {
			if (binding[i] != NULL)
				abort();
			binding[i] = sp;
			++sp->s_nkey;
		}
	}
}
