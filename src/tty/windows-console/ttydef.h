/*
 * Name:	MicroEMACS
 *		Digital ANSI terminal header file
 * Version:	29
 * Last edit:	05-Feb-86
 * By:		rex::conroy
 *		decvax!decwrl!dec-rhea!dec-rex!conroy
 */
#define	GOSLING	1			/* Compile in fancy display.	*/
#define	MEMMAP	0			/* Not memory mapped video.	*/

/*
 * Yes Bob, it's wrong for you.
 */
#define	NROW	50			/* Rows.			*/
#define	NCOL	132			/* Columns.			*/

/*
 * Special keys, as on the LK201, which is
 * a superset of the VT100. Originally I tried to keep the
 * numbers in LK201 escape sequence code, but it became too much
 * of a pain because of the keycodes greater than 31. 
 * The codes are all just redefinitions for the standard extra
 * key codes. Using the standard names ensures that the
 * LK201 codes land in the right place.
 *
 * Added keys for Windows/PC.
 */
#define	KUP	K01
#define	KDOWN	K02
#define	KLEFT	K03
#define	KRIGHT	K04
#define	KFIND	K05
#define	KINSERT	K06
#define	KREMOVE	K07
#define	KSELECT	K08
#define	KPREV	K09
#define	KNEXT	K0A
#define	KF4	K0B
#define	KF6	K0C
#define	KF7	K0D
#define	KF8	K0E
#define	KF9	K0F
#define	KF10	K10
#define	KF11	K11
#define	KF12	K12
#define	KF13	K13
#define	KF14	K14
#define	KHELP	K15
#define	KDO	K16
#define	KF17	K17
#define	KF18	K18
#define	KF19	K19
#define	KF20	K1A
#define	KPF1	K1B
#define	KPF2	K1C
#define	KPF3	K1D
#define	KPF4	K1E
