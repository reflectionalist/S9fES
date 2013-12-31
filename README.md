# Scheme 9 from Empty Space

## _A Portable Scheme Interpreter with a Unix Interface_

### By Nils M Holm, 2007-2013


## Introduction

S9fES is a mature, portable, and comprehensible public-domain interpreter for
R4RS Scheme offering

- decimal-based real number arithmetics;
- support for low-level Unix programming;
- cursor addressing with Curses;
- basic networking procedures;
- loads of useful library functions;
- an experimental Scheme-centric full-screen editor.

It is written in ANSI C (C89) and Scheme and it runs in many popular
environments, including *BSD, Linux, Plan 9, and the unmentionable horror
(although the Plan 9 and Horror versions probably exclude most of the above
goodies).

The S9fES code strives to be simple and comprehensible. It is particularly
interesting to people who want to (a) write Unix programs in a high-level
language (b) try Scheme without having to jump through too many hoops (c)
study the implementation of Scheme (in a language other than Scheme).

There is a book describing the implementation in detail. It is available in
print and PDF format. See http://t3x.org/s9book/

S9fES supports the following SRFI's:

- SRFI-0: feature-based conditional expansion (subset)
- SRFI-2: `and-let*` (subset)
- SRFI-22: running Scheme scripts on Unix
- SRFI-23: error reporting mechanism
- SRFI-30: nested multi-line comments
- `vector-copy` and `vector-append` from SRFI-43

### Note

If you are planning to use image files (which the S9 default setup does!), you
will have to disable both address space layout randomization (ASLR) and
position-independent code (PIE) for the S9 executable. This is because the
image file contains pointers to functions in the interpreter executable, and
either of the above techniques will invalidate these pointers, resulting in
fatal "wrong interpreter" errors when the S9 system starts up.

## Quick Start

You can run the interpreter in its build directory without installing it. Just
type <kbd>cc -Dunix -o s9 s9.c</kbd> and then <kbd>./s9</kbd>.

The S9 code is only loosely coupled to its Makefile, so in most cases running
<kbd>cc -Dunix -o s9 s9.c</kbd> or <kbd>8c -Dplan9 -o s9 s9.c</kbd> will
compile the interpreter just fine. (However, doing so will not include the
RealNum/Unix/Curses/Networking extensions.)

On most systems of the Unix family (plus Cygwin and MinGW), you can compile
and install S9fES by running <kbd>make install</kbd>.

Once installed, typing <kbd>s9</kbd> will start the interpreter.

<kbd>,h</kbd>  explains the online help system.

<kbd>,a</kbd>  returns a list of all help topics (long!).

If the above commands do not work, try <kbd>,l
[contrib/help.scm](contrib/help.scm)</kbd> first.


## Compiling and Installing

### Unix

To compile S9fES, run <kbd>make test</kbd> (this will also run the test suite
to make sure that the interpreter works properly). Running <kbd>make
tests</kbd> will run even more tests.

On 64-bit systems, you may want to add the `-DBITS_PER_WORD_64` definition to
the [Makefile](Makefile). Not doing so will probably still work, but result in
slightly worse _bignum_ performance.

You can install S9fES (including the goodies) on a Unix system by typing
<kbd>make install</kbd> as root, but before doing so, please edit at least the
`PREFIX` variable at the beginning of [Makefile](Makefile).  (Be sure to
re-compile S9fES (<kbd>make clean; make</kbd>) after changing `PREFIX`,
because it is used to set up some internal paths).

### Plan 9

To compile S9fES on Plan 9 from Bell Labs, just type <kbd>mk</kbd>, but note
that the Plan 9 port is rather experimental at this stage.  Above all, there
is currently no installation procedure.

### Manual Installation

To install S9fES manually,

- Compile s9 with a proper default library path (the one hardwired in
  [s9.h](s9.h) is probably not what you want). E.g.:

    <kbd>cc -o s9 -DDEFAULT_LIBRARY_PATH="\"</kbd><var>your-path</var><kbd>\""
    s9.c</kbd>

    A reasonable value for <var>your-path</var> would be, for example:

    <kbd>.:~/s9fes:/usr/local/share/s9fes</kbd>

    Security-sensitive people may consider removing the dot.

- Copy the s9 binary to a location where it can be executed (e.g.
  <kbd>/usr/local/bin</kbd>).

- Copy the file [s9.scm](s9.scm) to a publicly readable directory (default:
  <kbd>/usr/local/share/s9fes</kbd>). This directory will be referred to as
  <var>LIBDIR</var> in the following instructions. Note that this directory
  must be contained in DEFAULT_LIBRARY_PATH, as defined above.

- Compile a heap image and copy it to <var>LIBDIR</var>:

    <kbd>s9 -d s9.image</kbd>

    The image file must have the same base name as the interpreter or it will
    not be loaded. An image loads significantly faster than source code.

Optionally:

- Copy the content of the [lib](lib) directory to <var>LIBDIR</var>. This
  directory contains lots of useful Scheme functions.

- Copy the content of the [contrib](contrib) directory to <var>LIBDIR</var>.
  These files contain additional Scheme functions contributed by other authors
  or imported from various sources.

- Create a subdirectory named <kbd>help</kbd> in <var>LIBDIR</var> and copy
  the content of the [help](help) directory to
  <var>LIBDIR</var><kbd>/help</kbd>. These files are part of the interactive
  help system.

- Copy the _nroff(1)_ source code of the manual page [s9.1](s9.1) to section 1
  of your online manual (e.g. <kbd>/usr/local/man/man1</kbd>).  In case you
  are not running Unix, there is a pre-formatted copy in the file
  [s9.1.txt](s9.1.txt).


## Configuration

You may create the <kbd>S9FES_LIBRARY_PATH</kbd> environment variable and make
it point to <var>LIBDIR</var> as well as other directories that contain Scheme
programs. The variable may contain a single directory or a colon-separated
list of directories that will be searched in sequence for image files, library
files, and help pages (in help subdirectories of the given paths).

S9FES_LIBRARY_PATH overrides DEFAULT_LIBRARY_PATH, so all directories listed
in the latter should also be present in the former.

If you set up an _rc_-file in your home directory
(<kbd>$HOME/.s9fes/rc</kbd>), this file will be loaded before entering the
_REPL_. It will never be loaded when running programs non-interactively (with
the <kbd>-f</kbd> option).

To create an image file containing additional functionality, add the desired
options to the [config.scm](config.scm) file and run

<kbd>s9 -n -l config.scm -d s9.image</kbd>


## Getting Started

Typing <kbd>s9</kbd> will drop you into the read-eval-print loop of the
interpreter. You can run Scheme programs non-interactively by typing <kbd>s9
-f program.scm</kbd> at your shell prompt.

If you installed the extension library functions in <var>LIBDIR</var>, they
can be loaded by the `load-from-library` procedure or the <kbd>,l</kbd> (comma
ell) meta-command:

```
> ,l draw-tree
; loading from /usr/local/share/s9fes/draw-tree.scm
> (draw-tree '(a b c))
[o|o]---[o|o]---[o|/]
 |       |       |
 a       b       c
> 
```

Feel free to explore them.

Running <kbd>s9help </kbd><var>topic</var> on the shell prompt will print the
S9fES online help page about the given <var>topic</var>.


## Extended Interpreter

If compiled in, there are some extension procedures providing access to some
Unix system calls, some networking procedures, and a Curses interface. To
compile these extensions, uncomment the three `EXTRA_` lines in the
[Makefile](Makefile). (In fact, the Unix extensions are compiled in by
default.)


## Scheme Editor

The Scheme 9 Editor is started with the <kbd>s9e</kbd> command. The help page
is loaded by pressing <kbd>Ctrl</kbd> + <kbd>l</kbd> and then <kbd>h</kbd>.
See the **Usage** section at the end of the help file for instructions.

The most interesting feature of S9E is probably its interaction buffer. Type
<kbd>Ctrl</kbd> + <kbd>z</kbd> <kbd>s</kbd> to open an interaction buffer,
type any Scheme expression, move to its closing parenthesis and press
<kbd>Ctrl</kbd> + <kbd>j</kbd>.

To compile an S9E image, run <kbd>make s9e</kbd> or <kbd>make all</kbd>. To
install S9E, run <kbd>make install-all</kbd>. To try the editor without
installing, create an image (<kbd>make s9e</kbd>) and then run

<kbd>./s9 -i s9e-core prog/s9e1.scm</kbd>

Note that S9E is experimental at this point. Bug reports and patches are
welcome!


## Acknowledgements

I would like to thank the following people and organisations for patches, bug
reports, suggestions, hardware, access to hardware, etc:

Bakul Shah, Barak Pearlmutter, Blake McBride, Bsamograd (reddit), Dig1
(reddit), Dirk Lutzebaeck, Doug Currie, Mario Deilmann, Masaru KIMURA, Torsten
Leibold, and the Super Dimension Fortress (SDF.ORG).


## Contact

Nils M Holm < n m h @ t 3 x . o r g >

