/*
 * Spawn shell functionality for Windows.
 * 
 * Jörgen Sigvardsson <jorgen.sigvardsson@gmail.com>
 */
#include <Windows.h>
#include "def.h"

/*
 * This code does a one of 2 different
 * things, depending on what version of the shell
 * you are using. If you are using the C shell, which
 * implies that you are using job control, then MicroEMACS
 * moves the cursor to a nice place and sends itself a
 * stop signal. If you are using the Bourne shell it runs
 * a subshell using fork/exec. Bound to "C-C", and used
 * as a subcommand by "C-Z".
 */
spawncli(f, n, k)
{
	DWORD old_input_mode;
	DWORD old_output_mode;
	STARTUPINFOA si = {0};
	PROCESS_INFORMATION pi = {0};
	char* shell;
	
	si.cb = sizeof(si);

	/* Close down the current terminal mode */
	ttcolor(CTEXT);
	ttnowindow();
	ttflush();
	ttclose();

	/* Now the terminal is back into the same mode as prior to starting uemacs */

	/* Figure out what shell to execute */
	if (getenv("UEMACS_SHELL")) {
		/* If the user has set UEMACS_SHELL, then use it */
		shell = getenv("UEMACS_SHELL");
	} else if(getenv("ComSpec")) {
		/* If not, then use the regular shell */
		shell = getenv("ComSpec");
	} else {
		/* last resort! */
		shell = "cmd.exe";
	}

	/* Start the child process. */
	if (!CreateProcessA(
		NULL,           /* No module name (use command line) */
		shell,          /* The selected shell*/
		NULL,           /* Process handle not inheritable */
		NULL,           /* Thread handle not inheritable */
		FALSE,          /* Set handle inheritance to FALSE */
		0,              /* No creation flags */
		NULL,           /* Use parent's environment block */
		NULL,           /* Use parent's starting directory */
		&si,            /* Pointer to STARTUPINFO structure */
		&pi)            /* Pointer to PROCESS_INFORMATION structure */
		) {
		eprintf("Failed to create process");
		return FALSE;
	}

	/* Wait until child process exits. */
	WaitForSingleObject(pi.hProcess, INFINITE);

	/* Close process and thread handles. */
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	sgarbf = TRUE;				/* Force repaint.	*/
	ttopen();

	return TRUE;
}
