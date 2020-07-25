/*
 * Name:	MicroEMACS
 *		Symbol table stuff.
 * Version:	29
 * Last edit:	05-Feb-86
 * By:		rex::conroy
 *		decvax!decwrl!dec-rhea!dec-rex!conroy
 *
 * Symbol tables, and keymap setup.
 * The terminal specific parts of building the
 * keymap has been moved to a better place.
 */
#include	"def.h"

#define	DIRLIST	0			/* Disarmed!			*/

/*
 * Defined by "main.c".
 */
extern	int	ctrlg();		/* Abort out of things		*/
extern	int	quit();			/* Quit				*/
extern	int	ctlxlp();		/* Begin macro			*/
extern	int	ctlxrp();		/* End macro			*/
extern	int	ctlxe();		/* Execute macro		*/
extern	int	jeffexit();		/* Jeff Lomicka style exit.	*/
extern  int	showversion();		/* Show version numbers, etc.	*/

/*
 * Defined by "search.c".
 */
extern	int	forwsearch();		/* Search forward		*/
extern	int	backsearch();		/* Search backwards		*/
extern  int	searchagain();		/* Repeat last search command	*/
extern  int	forwisearch();		/* Incremental search forward	*/
extern  int	backisearch();		/* Incremental search backwards	*/
extern  int	queryrepl();		/* Query replace		*/

/*
 * Defined by "basic.c".
 */
extern	int	gotobol();		/* Move to start of line	*/
extern	int	backchar();		/* Move backward by characters	*/
extern	int	gotoeol();		/* Move to end of line		*/
extern	int	forwchar();		/* Move forward by characters	*/
extern	int	gotobob();		/* Move to start of buffer	*/
extern	int	gotoeob();		/* Move to end of buffer	*/
extern	int	forwline();		/* Move forward by lines	*/
extern	int	backline();		/* Move backward by lines	*/
extern	int	forwpage();		/* Move forward by pages	*/
extern	int	backpage();		/* Move backward by pages	*/
extern	int	setmark();		/* Set mark			*/
extern	int	swapmark();		/* Swap "." and mark		*/
extern	int	gotoline();		/* Go to a specified line.	*/

/*
 * Defined by "buffer.c".
 */
extern	int	listbuffers();		/* Display list of buffers	*/
extern	int	usebuffer();		/* Switch a window to a buffer	*/
extern	int	killbuffer();		/* Make a buffer go away.	*/

#if	DIRLIST
/*
 * Defined by "dirlist.c".
 */
extern	int	dirlist();		/* Directory list.		*/
#endif

/*
 * Defined by "display.c".
 */
extern  int	readmsg();		/* Read next line of message.	*/

/*
 * Defined by "file.c".
 */
extern	int	fileread();		/* Get a file, read only	*/
extern	int	filevisit();		/* Get a file, read write	*/
extern	int	filewrite();		/* Write a file			*/
extern	int	filesave();		/* Save current file		*/
extern	int	filename();		/* Adjust file name		*/

/*
 * Defined by "random.c".
 */
extern	int	selfinsert();		/* Insert character		*/
extern	int	showcpos();		/* Show the cursor position	*/
extern	int	twiddle();		/* Twiddle characters		*/
extern	int	quote();		/* Insert literal		*/
extern	int	openline();		/* Open up a blank line		*/
extern	int	newline();		/* Insert CR-LF			*/
extern	int	deblank();		/* Delete blank lines		*/
extern	int	indent();		/* Insert CR-LF, then indent	*/
extern	int	forwdel();		/* Forward delete		*/
extern	int	backdel();		/* Backward delete		*/
extern	int	killline();		/* Kill forward			*/
extern	int	yank();			/* Yank back from killbuffer.	*/

/*
 * Defined by "region.c".
 */
extern	int	killregion();		/* Kill region.			*/
extern	int	copyregion();		/* Copy region to kill buffer.	*/
extern	int	lowerregion();		/* Lower case region.		*/
extern	int	upperregion();		/* Upper case region.		*/

/*
 * Defined by "spawn.c".
 */
extern	int	spawncli();		/* Run CLI in a subjob.		*/

/*
 * Defined by "window.c".
 */
extern	int	reposition();		/* Reposition window		*/
extern	int	refresh();		/* Refresh the screen		*/
extern	int	nextwind();		/* Move to the next window	*/
extern  int	prevwind();		/* Move to the previous window	*/
extern	int	mvdnwind();		/* Move window down		*/
extern	int	mvupwind();		/* Move window up		*/
extern	int	onlywind();		/* Make current window only one	*/
extern	int	splitwind();		/* Split current window		*/
extern	int	enlargewind();		/* Enlarge display window.	*/
extern	int	shrinkwind();		/* Shrink window.		*/

/*
 * Defined by "word.c".
 */
extern	int	backword();		/* Backup by words		*/
extern	int	forwword();		/* Advance by words		*/
extern	int	upperword();		/* Upper case word.		*/
extern	int	lowerword();		/* Lower case word.		*/
extern	int	capword();		/* Initial capitalize word.	*/
extern	int	delfword();		/* Delete forward word.		*/
extern	int	delbword();		/* Delete backward word.	*/

/*
 * Defined by "extend.c".
 */
extern	int	extend();		/* Extended commands.		*/
extern	int	help();			/* Help key.			*/
extern	int	bindtokey();		/* Modify key bindings.		*/
extern	int	wallchart();		/* Make wall chart.		*/

typedef	struct	{
	short	k_key;			/* Key to bind.			*/
	int	(*k_funcp)();		/* Function.			*/
	char	*k_name;		/* Function name string.	*/
}	KEY;

/*
 * Default key binding table. This contains
 * the function names, the symbol table name, and (possibly)
 * a key binding for the builtin functions. There are no
 * bindings for C-U or C-X. These are done with special
 * code, but should be done normally.
 */
KEY	key[] = {
	KCTRL|'@',	setmark,	"set-mark",
	KCTRL|'A',	gotobol,	"goto-bol",
	KCTRL|'B',	backchar,	"back-char",
	KCTRL|'C',	spawncli,	"spawn-cli",
	KCTRL|'D',	forwdel,	"forw-del-char",
	KCTRL|'E',	gotoeol,	"goto-eol",
	KCTRL|'F',	forwchar,	"forw-char",
	KCTRL|'G',	ctrlg,		"abort",
	KCTRL|'H',	backdel,	"back-del-char",
	KCTRL|'I',	selfinsert,	"ins-self",
	KCTRL|'J',	indent,		"ins-nl-and-indent",
	KCTRL|'K',	killline,	"kill-line",
	KCTRL|'L',	refresh,	"refresh",
	KCTRL|'M',	newline,	"ins-nl",
	KCTRL|'N',	forwline,	"forw-line",
	KCTRL|'O',	openline,	"ins-nl-and-backup",
	KCTRL|'P',	backline,	"back-line",
	KCTRL|'Q',	quote,		"quote",
	KCTRL|'R',	backisearch,	"back-i-search",
	KCTRL|'S',	forwisearch,	"forw-i-search",
	KCTRL|'T',	twiddle,	"twiddle",
	KCTRL|'V',	forwpage,	"forw-page",
	KCTRL|'W',	killregion,	"kill-region",
	KCTRL|'Y',	yank,		"yank",
	KCTRL|'Z',	jeffexit,	"jeff-exit",
	KCTLX|KCTRL|'B',listbuffers,	"display-buffers",
	KCTLX|KCTRL|'C',quit,		"quit",
#if	DIRLIST
	KCTLX|KCTRL|'D',dirlist,	"display-directory",
#endif
	KCTLX|KCTRL|'F',filename,	"set-file-name",
	KCTLX|KCTRL|'L',lowerregion,	"lower-region",
	KCTLX|KCTRL|'N',mvdnwind,	"down-window",
	KCTLX|KCTRL|'O',deblank,	"del-blank-lines",
	KCTLX|KCTRL|'P',mvupwind,	"up-window",
	KCTLX|KCTRL|'R',fileread,	"file-read",
	KCTLX|KCTRL|'S',filesave,	"file-save",
	KCTLX|KCTRL|'U',upperregion,	"upper-region",
	KCTLX|KCTRL|'V',filevisit,	"file-visit",
	KCTLX|KCTRL|'W',filewrite,	"file-write",
	KCTLX|KCTRL|'X',swapmark,	"swap-dot-and-mark",
	KCTLX|KCTRL|'Z',shrinkwind,	"shrink-window",
	KCTLX|'=',	showcpos,	"display-position",
	KCTLX|'(',	ctlxlp,		"start-macro",
	KCTLX|')',	ctlxrp,		"end-macro",
	KCTLX|'1',	onlywind,	"only-window",
	KCTLX|'2',	splitwind,	"split-window",
	KCTLX|'B',	usebuffer,	"use-buffer",
	KCTLX|'E',	ctlxe,		"execute-macro",
	KCTLX|'G',	gotoline,	"goto-line",
	KCTLX|'K',	killbuffer,	"kill-buffer",
	KCTLX|'N',	nextwind,	"forw-window",
	KCTLX|'P',	prevwind,	"back-window",
	KCTLX|'Z',	enlargewind,	"enlarge-window",
	KMETA|KCTRL|'H',delbword,	"back-del-word",
	KMETA|KCTRL|'R',readmsg,	"display-message",
	KMETA|KCTRL|'V',showversion,	"display-version",
	KMETA|'!',	reposition,	"reposition-window",
	KMETA|'>',	gotoeob,	"goto-eob",
	KMETA|'<',	gotobob,	"goto-bob",
	KMETA|'%',	queryrepl,	"query-replace",
	KMETA|'B',	backword,	"back-word",
	KMETA|'C',	capword,	"cap-word",
	KMETA|'D',	delfword,	"forw-del-word",
	KMETA|'F',	forwword,	"forw-word",
	KMETA|'L',	lowerword,	"lower-word",
	KMETA|'R',	backsearch,	"back-search",
	KMETA|'S',	forwsearch,	"forw-search",
	KMETA|'U',	upperword,	"upper-word",
	KMETA|'V',	backpage,	"back-page",
	KMETA|'W',	copyregion,	"copy-region",
	KMETA|'X',	extend,		"extended-command",
	-1,		searchagain,	"search-again",
	-1,		help,		"help",
	-1,		wallchart,	"display-bindings",
	-1,		bindtokey,	"bind-to-key"
};

#define	NKEY	(sizeof(key) / sizeof(key[0]))

/*
 * Symbol table lookup.
 * Return a pointer to the SYMBOL node, or NULL if
 * the symbol is not found.
 */
SYMBOL	*
symlookup(cp)
register char	*cp;
{
	register SYMBOL	*sp;

	sp = symbol[symhash(cp)];
	while (sp != NULL) {
		if (strcmp(cp, sp->s_name) == 0)
			return (sp);
		sp = sp->s_symp;
	}
	return (NULL);
}

/*
 * Take a string, and compute the symbol table
 * bucket number. This is done by adding all of the characters
 * together, and taking the sum mod NSHASH. The string probably
 * should not contain any GR characters; if it does the "*cp"
 * may get a nagative number on some machines, and the "%"
 * will return a negative number!
 */
symhash(cp)
register char	*cp;
{
	register int	c;
	register int	n;

	n = 0;
	while ((c = *cp++) != 0)
		n += c;
	return (n % NSHASH);
}

/*
 * Build initial keymap. The funny keys
 * (commands, odd control characters) are mapped using
 * a big table and calls to "keyadd". The printing characters
 * are done with some do-it-yourself handwaving. The terminal
 * specific keymap initialization code is called at the
 * very end to finish up. All errors are fatal.
 */
keymapinit()
{
	register SYMBOL	*sp;
	register KEY	*kp;
	register int	i;
	register int	hash;

	for (i=0; i<NKEYS; ++i)
		binding[i] = NULL;
	for (kp = &key[0]; kp < &key[NKEY]; ++kp)
		keyadd(kp->k_key, kp->k_funcp, kp->k_name);
	keydup(KCTLX|KCTRL|'G',	"abort");
	keydup(KMETA|KCTRL|'G',	"abort");
	keydup(0x7F,		"back-del-char");
	keydup(KCTLX|'R',	"back-i-search");
	keydup(KCTLX|'S',	"forw-i-search");
	keydup(KMETA|'.',	"set-mark");
	keydup(KMETA|'Q',	"quote");
	keydup(KMETA|0x7F,	"back-del-word");
	/*
	 * Should be bound by "tab" already.
	 */
	if ((sp=symlookup("ins-self")) == NULL)
		abort();
	for (i=0x20; i<0x7F; ++i) {
		if (binding[i] != NULL)
			abort();
		binding[i] = sp;
		++sp->s_nkey;
	}
	ttykeymapinit();
}

/*
 * Create a new builtin function "name"
 * with function "funcp". If the "new" is a real
 * key, bind it as a side effect. All errors
 * are fatal.
 */
keyadd(new, funcp, name)
int	(*funcp)();
char	*name;
{
	register SYMBOL	*sp;
	register int	hash;

	if ((sp=(SYMBOL *)malloc(sizeof(SYMBOL))) == NULL)
		abort();
	hash = symhash(name);
	sp->s_symp = symbol[hash];
	symbol[hash] = sp;
	sp->s_nkey = 0;
	sp->s_name = name;
	sp->s_funcp = funcp;
	if (new >= 0) {				/* Bind this key.	*/
		if (binding[new] != NULL)
			abort();
		binding[new] = sp;
		++sp->s_nkey;
	}
}

/*
 * Bind key "new" to the existing
 * routine "name". If the name cannot be found,
 * or the key is already bound, abort.
 */
keydup(new, name)
register int	new;
char		*name;
{
	register SYMBOL	*sp;

	if (binding[new]!=NULL || (sp=symlookup(name))==NULL)
		abort();
	binding[new] = sp;
	++sp->s_nkey;
}
