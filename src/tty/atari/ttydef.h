/*
 * Name:	MicroEMACS
 *		Atari 520ST header file.
 * Version:	30
 * Last edit:	22-Feb-86
 * By:		rex::conroy
 *		decvax!decwrl!dec-rhea!dec-rex!conroy
 */
#define	GOSLING	1			/* Use fancy redisplay.		*/
#define	MEMMAP	0			/* Not memory mapped video.	*/

#define	NROW	50			/* The "50" is big enough to	*/
#define	NCOL	80			/* deal with the "hi50" screen.	*/

/*
 * Special keys.
 */
#define	KF1	K01
#define	KF2	K02
#define	KF3	K03
#define	KF4	K04
#define	KF5	K05
#define	KF6	K06
#define	KF7	K07
#define	KF8	K08
#define	KF9	K09
#define	KF10	K0A
#define	KHELP	K0B
#define	KUNDO	K0C
#define	KINSERT	K0D
#define	KUP	K0E
#define	KCLEAR	K0F
#define	KLEFT	K10
#define	KDOWN	K11
#define	KRIGHT	K12
