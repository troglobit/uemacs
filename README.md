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
directory, which in part is based on `sys/ultrix`.

The build system has been given a boost with GNU Autoconf and Automake.
Run `./autogen.sh` when checking out the sources from GIT to create the
`configure` script and `Makefile.in`'s.


TODO
----

There are still code paths that are not suited for a modern compiler,
even when running it in C89 mode.  The most glaring remaing problem
right now is old-style `/* VARARGS1 */` and `/* VARARGS3 */` code in
`src/echo.c` that causes segfault.  Figuring out the least intrusive
way to patch that to use post-1989 `stdarg.h` is the next challenge.
