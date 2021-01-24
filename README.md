MicroEMACS v30
==============

This is the oldest version of MicroEMACS by Dave Conroy, predating any
of Daniel Lawrence's changes.  It's version 30, and not 3.0.  Maybe
patterned after the original EMACS sequential version numbers.

Sources from Lars Brinkhoff's Historical Emacs Software Preservation
repository at GitHub: https://github.com/larsbrinkhoff/emacs-history/

Great care has been taken to not change the original code, only the most
obvious bugs have been handled, as gracefully as possible.  The rest is
clever hiding of warnings and use of C89.

Only addition to build on modern UNIX/Linux systems is the `sys/unix/`
directory, which in part is based on `sys/ultrix`.  There is also a
native Windows port, see `build-win.cmd` for details.

The build system has been given a boost with GNU Autoconf and Automake.
Run `./autogen.sh` when checking out the sources from GIT to create the
`configure` script and `Makefile.in`'s.


Porting Notes
-------------

This port to was made by Joachim Wiberg (UNIX) and Jörgen Sigvardsson
(Windows).  Other ports or bug fixes are most welcome, as long as they
stay in the spirit of the original MicroEMACS.

