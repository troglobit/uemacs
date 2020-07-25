/*
 * Name:	MicroEMACS
 *		Terminal independent keyboard handling.
 * Version:	29
 * Last edit:	05-Feb-86
 * By:		rex::conroy
 *		decvax!decwrl!dec-rhea!dec-rex!conroy
 */
#include	"def.h"

/*
 * Read in a key, doing the terminal
 * independent prefix handling. The terminal specific
 * "getkbd" routine gets the first swing, and may return
 * one of the special codes used by the special keys
 * on the keyboard. The "getkbd" routine returns the
 * C0 controls as received; this routine moves them to
 * the right spot in 11 bit code.
 */
getkey()
{
	register int	c;

	c = getkbd();
	if (c == METACH)			/* M-			*/
		c = KMETA | getctl();
	else if (c == CTRLCH)			/* C-			*/
		c = KCTRL | getctl();
	else if (c == CTMECH)			/* C-M-			*/
		c = KCTRL | KMETA | getctl();
	else if (c>=0x00 && c<=0x1F)		/* Relocate control.	*/
		c = KCTRL | (c+'@');
	if (c == (KCTRL|'X'))			/* C-X			*/
		c = KCTLX | getctl();
	return (c);
}

/*
 * Used above.
 */
getctl()
{
	register int	c;

	c = ttgetc();
	if (ISLOWER(c) != FALSE)
		c = TOUPPER(c);
	if (c>=0x00 && c<=0x1F)			/* Relocate control.	*/
		c = KCTRL | (c+'@');
	return (c);
}

/*
 * Transform a key code into a name,
 * using a table for the special keys and combination
 * of some hard code and some general processing for
 * the rest. None of this code is terminal specific any
 * more. This makes adding keys easier.
 */
keyname(cp, k)
register char	*cp;
register int	k;
{
	register char	*np;
	char		nbuf[3];

	static	char	hex[] = {
		'0',	'1',	'2',	'3',
		'4',	'5',	'6',	'7',
		'8',	'9',	'A',	'B',
		'C',	'D',	'E',	'F'
	};

	if ((k&KCTLX) != 0) {			/* C-X prefix.		*/
		*cp++ = 'C';
		*cp++ = '-';
		*cp++ = 'X';
		*cp++ = ' ';
		k &= ~KCTLX;
	}
	if ((k&KCHAR)>=KFIRST && (k&KCHAR)<=KLAST) {
		if ((np=keystrings[(k&KCHAR)-KFIRST]) != NULL) {
			if ((k&KCTRL) != 0) {
				*cp++ = 'C';
				*cp++ = '-';
			}
			if ((k&KMETA) != 0) {
				*cp++ = 'M';
				*cp++ = '-';
			}
			strcpy(cp, np);
			return;
		}
	}
	if ((k&~KMETA) == (KCTRL|'I'))		/* Some specials.	*/
		np = "Tab";
	else if ((k&~KMETA) == (KCTRL|'M'))
		np = "Return";
	else if ((k&~KMETA) == (KCTRL|'H'))
		np = "Backspace";
	else if ((k&~KMETA) == ' ')
		np = "Space";
	else if ((k&~KMETA) == 0x7F)
		np = "Rubout";
	else {
		if ((k&KCTRL) != 0) {		/* Add C- mark.		*/
			*cp++ = 'C';
			*cp++ = '-';
		}
		np = &nbuf[0];
		if (((k&KCHAR)>=0x20 && (k&KCHAR)<=0x7E)
		||  ((k&KCHAR)>=0xA0 && (k&KCHAR)<=0xFE)) {
			nbuf[0] = k&KCHAR;	/* Graphic.		*/
			nbuf[1] = 0;
		} else {			/* Non graphic.		*/
			nbuf[0] = hex[(k>>4)&0x0F];
			nbuf[1] = hex[k&0x0F];
			nbuf[2] = 0;
		}
	}
	if ((k&KMETA) != 0) {			/* Add M- mark.		*/
		*cp++ = 'M';
		*cp++ = '-';
	}
	strcpy(cp, np);
}
