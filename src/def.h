/*
 * Name:	MicroEMACS
 *		Common header file.
 * Version:	29
 * Last edit:	14-Feb-86
 * By:		rex::conroy
 *		decvax!decwrl!dec-rhea!dec-rex!conroy
 * 
 * This file is the general header file for all parts
 * of the MicroEMACS display editor. It contains all of the
 * general definitions and macros. It also contains some
 * conditional compilation flags. All of the per-system and
 * per-terminal definitions are in special header files.
 * The most common reason to edit this file would be to zap
 * the definition of CVMVAS or BACKUP.
 */
#include	"sysdef.h"		/* Order is critical.		*/
#include	"ttydef.h"
#include	<stdio.h>

#define	CVMVAS	1			/* C-V, M-V work in pages.	*/
#define	BACKUP	0			/* Make backup file.		*/

/*
 * Table sizes, etc.
 */
#define	NSHASH	31			/* Symbol table hash size.	*/
#define	NFILEN	80			/* Length, file name.		*/
#define	NBUFN	16			/* Length, buffer name.		*/
#define	NLINE	256			/* Length, line.		*/
#define	NKBDM	256			/* Length, keyboard macro.	*/
#define NMSG	512			/* Length, message buffer.	*/
#define	NPAT	80			/* Length, pattern.		*/
#define	HUGE	1000			/* A rather large number.	*/
#define NSRCH	128			/* Undoable search commands.	*/
#define	NXNAME	64			/* Length, extended command.	*/

/*
 * Universal.
 */
#define	FALSE	0			/* False, no, bad, etc.		*/
#define	TRUE	1			/* True, yes, good, etc.	*/
#define	ABORT	2			/* Death, ^G, abort, etc.	*/

/*
 * These flag bits keep track of
 * some aspects of the last command. The CFCPCN
 * flag controls goal column setting. The CFKILL
 * flag controls the clearing versus appending
 * of data in the kill buffer.
 */
#define	CFCPCN	0x0001			/* Last command was C-P, C-N	*/
#define	CFKILL	0x0002			/* Last command was a kill	*/

/*
 * File I/O.
 */
#define	FIOSUC	0			/* Success.			*/
#define	FIOFNF	1			/* File not found.		*/
#define	FIOEOF	2			/* End of file.			*/
#define	FIOERR	3			/* Error.			*/

/*
 * Directory I/O.
 */
#define	DIOSUC	0			/* Success.			*/
#define	DIOEOF	1			/* End of file.			*/
#define	DIOERR	2			/* Error.			*/

/*
 * Display colors.
 */
#define	CNONE	0			/* Unknown color.		*/
#define	CTEXT	1			/* Text color.			*/
#define	CMODE	2			/* Mode line color.		*/

/*
 * Flags for "eread".
 */
#define	EFNEW	0x0001			/* New prompt.			*/
#define	EFAUTO	0x0002			/* Autocompletion enabled.	*/
#define	EFCR	0x0004			/* Echo CR at end; last read.	*/

/*
 * Keys are represented inside using an 11 bit
 * keyboard code. The transformation between the keys on
 * the keyboard and 11 bit code is done by terminal specific
 * code in the "kbd.c" file. The actual character is stored
 * in 8 bits (DEC multinationals work); there is also a control
 * flag KCTRL, a meta flag KMETA, and a control-X flag KCTLX.
 * ASCII control characters are always represented using the
 * KCTRL form. Although the C0 control set is free, it is
 * reserved for C0 controls because it makes the communication
 * between "getkey" and "getkbd" easier. The funny keys get
 * mapped into the C1 control area.
 */
#define	NKEYS	2048			/* 11 bit code.			*/

#define	METACH	0x1B			/* M- prefix,   Control-[, ESC	*/
#define	CTMECH	0x1C			/* C-M- prefix, Control-\	*/
#define	EXITCH	0x1D			/* Exit level,  Control-]	*/
#define	CTRLCH	0x1E			/* C- prefix,	Control-^	*/
#define	HELPCH	0x1F			/* Help key,    Control-_	*/

#define	KCHAR	0x00FF			/* The basic character code.	*/
#define	KCTRL	0x0100			/* Control flag.		*/
#define	KMETA	0x0200			/* Meta flag.			*/
#define	KCTLX	0x0400			/* Control-X flag.		*/

#define	KFIRST	0x0080			/* First special.		*/
#define	KLAST	0x009F			/* Last special.		*/

#define	KRANDOM	0x0080			/* A "no key" code.		*/
#define	K01	0x0081			/* Use these names to define	*/
#define	K02	0x0082			/* the special keys on your	*/
#define	K03	0x0083			/* terminal.			*/
#define	K04	0x0084
#define	K05	0x0085
#define	K06	0x0086
#define	K07	0x0087
#define	K08	0x0088
#define	K09	0x0089
#define	K0A	0x008A
#define	K0B	0x008B
#define	K0C	0x008C
#define	K0D	0x008D
#define	K0E	0x008E
#define	K0F	0x008F
#define	K10	0x0090
#define	K11	0x0091
#define	K12	0x0092
#define	K13	0x0093
#define	K14	0x0094
#define	K15	0x0095
#define	K16	0x0096
#define	K17	0x0097
#define	K18	0x0098
#define	K19	0x0099
#define	K1A	0x009A
#define	K1B	0x009B
#define	K1C	0x009C
#define	K1D	0x009D
#define	K1E	0x009E
#define	K1F	0x009F

/*
 * These flags, and the macros below them,
 * make up a do-it-yourself set of "ctype" macros that
 * understand the DEC multinational set, and let me ask
 * a slightly different set of questions.
 */
#define	_W	0x01			/* Word.			*/
#define	_U	0x02			/* Upper case letter.		*/
#define	_L	0x04			/* Lower case letter.		*/
#define	_C	0x08			/* Control.			*/

#define	ISWORD(c)	((cinfo[(c)]&_W)!=0)
#define	ISCTRL(c)	((cinfo[(c)]&_C)!=0)
#define	ISUPPER(c)	((cinfo[(c)]&_U)!=0)
#define	ISLOWER(c)	((cinfo[(c)]&_L)!=0)
#define	TOUPPER(c)	((c)-0x20)
#define	TOLOWER(c)	((c)+0x20)

/*
 * The symbol table links editing functions
 * to names. Entries in the key map point at the symbol
 * table entry. A reference count is kept, but it is
 * probably next to useless right now. The old type code,
 * which was not being used and probably not right
 * anyway, is all gone.
 */
typedef	struct	SYMBOL {
	struct	SYMBOL *s_symp;		/* Hash chain.			*/
	short	s_nkey;			/* Count of keys bound here.	*/
	char	*s_name;		/* Name.			*/
	int	(*s_funcp)();		/* Function.			*/
}	SYMBOL;

/*
 * There is a window structure allocated for
 * every active display window. The windows are kept in a
 * big list, in top to bottom screen order, with the listhead at
 * "wheadp". Each window contains its own values of dot and mark.
 * The flag field contains some bits that are set by commands
 * to guide redisplay; although this is a bit of a compromise in
 * terms of decoupling, the full blown redisplay is just too
 * expensive to run for every input character. 
 */
typedef	struct	WINDOW {
	struct	WINDOW *w_wndp;		/* Next window			*/
	struct	BUFFER *w_bufp;		/* Buffer displayed in window	*/
	struct	LINE *w_linep;		/* Top line in the window	*/
	struct	LINE *w_dotp;		/* Line containing "."		*/
	short	w_doto;			/* Byte offset for "."		*/
	struct	LINE *w_markp;		/* Line containing "mark"	*/
	short	w_marko;		/* Byte offset for "mark"	*/
	char	w_toprow;		/* Origin 0 top row of window	*/
	char	w_ntrows;		/* # of rows of text in window	*/
	char	w_force;		/* If NZ, forcing row.		*/
	char	w_flag;			/* Flags.			*/
}	WINDOW;

/*
 * Window flags are set by command processors to
 * tell the display system what has happened to the buffer
 * mapped by the window. Setting "WFHARD" is always a safe thing
 * to do, but it may do more work than is necessary. Always try
 * to set the simplest action that achieves the required update.
 * Because commands set bits in the "w_flag", update will see
 * all change flags, and do the most general one.
 */
#define	WFFORCE	0x01			/* Force reframe.		*/
#define	WFMOVE	0x02			/* Movement from line to line.	*/
#define	WFEDIT	0x04			/* Editing within a line.	*/
#define	WFHARD	0x08			/* Better to a full display.	*/
#define	WFMODE	0x10			/* Update mode line.		*/

/*
 * Text is kept in buffers. A buffer header, described
 * below, exists for every buffer in the system. The buffers are
 * kept in a big list, so that commands that search for a buffer by
 * name can find the buffer header. There is a safe store for the
 * dot and mark in the header, but this is only valid if the buffer
 * is not being displayed (that is, if "b_nwnd" is 0). The text for
 * the buffer is kept in a circularly linked list of lines, with
 * a pointer to the header line in "b_linep".
 */
typedef	struct	BUFFER {
	struct	BUFFER *b_bufp;		/* Link to next BUFFER		*/
	struct	LINE *b_dotp;		/* Link to "." LINE structure	*/
	short	b_doto;			/* Offset of "." in above LINE	*/
	struct	LINE *b_markp;		/* The same as the above two,	*/
	short	b_marko;		/* but for the "mark"		*/
	struct	LINE *b_linep;		/* Link to the header LINE	*/
	char	b_nwnd;			/* Count of windows on buffer	*/
	char	b_flag;			/* Flags			*/
	char	b_fname[NFILEN];	/* File name			*/
	char	b_bname[NBUFN];		/* Buffer name			*/
}	BUFFER;

#define	BFCHG	0x01			/* Changed.			*/
#define	BFBAK	0x02			/* Need to make a backup.	*/

/*
 * This structure holds the starting position
 * (as a line/offset pair) and the number of characters in a
 * region of a buffer. This makes passing the specification
 * of a region around a little bit easier.
 * There have been some complaints that the short in this
 * structure is wrong; that a long would be more appropriate.
 * I'll awat more comments from the folks with the little
 * machines; I have a VAX, and everything fits.
 */
typedef	struct	{
	struct	LINE *r_linep;		/* Origin LINE address.		*/
	short	r_offset;		/* Origin LINE offset.		*/
	short	r_size;			/* Length in characters.	*/
}	REGION;

/*
 * All text is kept in circularly linked
 * lists of "LINE" structures. These begin at the
 * header line (which is the blank line beyond the
 * end of the buffer). This line is pointed to by
 * the "BUFFER". Each line contains a the number of
 * bytes in the line (the "used" size), the size
 * of the text array, and the text. The end of line
 * is not stored as a byte; it's implied. Future
 * additions will include update hints, and a
 * list of marks into the line.
 */
typedef	struct	LINE {
	struct	LINE *l_fp;		/* Link to the next line	*/
	struct	LINE *l_bp;		/* Link to the previous line	*/
	short	l_size;			/* Allocated size		*/
	short	l_used;			/* Used size			*/
#if	PCC
	char	l_text[1];		/* A bunch of characters.	*/
#else
	char	l_text[];		/* A bunch of characters.	*/
#endif
}	LINE;

/*
 * The rationale behind these macros is that you
 * could (with some editing, like changing the type of a line
 * link from a "LINE *" to a "REFLINE", and fixing the commands
 * like file reading that break the rules) change the actual
 * storage representation of lines to use something fancy on
 * machines with small address spaces.
 */
#define	lforw(lp)	((lp)->l_fp)
#define	lback(lp)	((lp)->l_bp)
#define	lgetc(lp, n)	((lp)->l_text[(n)]&0xFF)
#define	lputc(lp, n, c)	((lp)->l_text[(n)]=(c))
#define	llength(lp)	((lp)->l_used)

/*
 * Externals.
 */
extern	int	thisflag;
extern	int	lastflag;
extern	int	curgoal;
extern	int	epresf;
extern	int	sgarbf;
extern	WINDOW	*curwp;
extern	BUFFER	*curbp;
extern	WINDOW	*wheadp;
extern	BUFFER	*bheadp;
extern	BUFFER	*blistp;
extern	short	kbdm[];
extern	short	*kbdmip;
extern	short	*kbdmop;
extern	char	pat[];
extern	SYMBOL	*symbol[];
extern	SYMBOL	*binding[];
extern	BUFFER	*bfind();
extern	BUFFER	*bcreate();
extern	WINDOW	*wpopup();
extern	LINE	*lalloc();
extern  int	nrow;
extern  int	ncol;
extern	char	*version[];
extern	int	ttrow;
extern	int	ttcol;
extern	int	tceeol;
extern	int	tcinsl;
extern	int	tcdell;
extern	char	cinfo[];
extern	char	*keystrings[];
extern	SYMBOL	*symlookup();
extern	int	nmsg;
extern	int	curmsgf;
extern	int	newmsgf;
extern	char	msg[];

/*
 * Standard I/O.
 */
extern	char	*malloc();
extern	char	*strcpy();
extern	char	*strcat();
