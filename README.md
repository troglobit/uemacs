MicroEMACS v30
==============

This is the oldest version of MicroEMACS by Dave Conroy, predating any
of Daniel Lawrence's changes.  It's version 30, and not 3.0.  Maybe
patterned after the original EMACS sequential version numbers.

Sources from Lars Brinkhoff's Historical Emacs Software Preservation
repository at GitHub: https://github.com/larsbrinkhoff/emacs-history/

Great care has been taken to not change the original code, only the most
obvious bugs have been handled, as gracefully as possible.

Only addition to build on modern UNIX/Linux systems is the `sys/unix/`
directory, which in part is based on `sys/ultrix`.

A top-level Makefile has been added to make building easier.  If you
need to build for any of the legacy targets, you need to override the
`SYS` variable from your `$SHELL`.

