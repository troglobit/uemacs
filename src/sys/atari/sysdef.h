/*
 * Name:	MicroEMACS
 *		Atari 520ST header file.
 * Version:	30
 * Last edit:	22-Feb-86
 * By:		rex::conroy
 *		decvax!decwrl!dec-rhea!dec-rex!conroy
 */
#define	PCC	1			/* "[]" gets an error.		*/
#define	KBLOCK	512			/* Kill grow.			*/
#define	GOOD	0			/* Good exit status.		*/

/*
 * For "(void)" casts.
 */
typedef	int	void;

/*
 * Macros used by the buffer name making code.
 * Start at the end of the file name, scan to the left
 * until BDC1 (or BDC2, if defined) is reached. The buffer
 * name starts just to the right of that location, and
 * stops at end of string (or at the next BDC3 character,
 * if defined). BDC2 and BDC3 are mainly for VMS.
 */
#define	BDC1	'\\'			/* Buffer names.		*/
#define	BDC2	':'
