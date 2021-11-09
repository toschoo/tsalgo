# TS Algorithms (TSALGO)

This is a collection of fundamental algorithms
often used and often implemented throughout all
kinds of software projects.

Most programmers have probably already implemented
several of the algorithms provided here --
perhaps more than once.
This collection, hence, is nothing
ground-breaking. Even worse, there may
be better, in particular, faster or
more resource-friendly implementations of
the algorithms out there. So, why publishing
this code? 

This code has grown over the years
in real-world projects or for self-education.
I now publish it piece by piece making sure
that the library as a whole forms a consistent
and useful tool for practical applications
and for (self-)education. To this end, the code is
commented including references to literature.

I hope people could learn something from it
and that mistakes (that are certainly there)
will not mislead people. 
Feedback, corrections and improvements are 
therefore appreciated.

Currently the library contains:
- a simple list type
- sorting algorithms for
  + buffers
  + lists
  + files ("external sorting")
- an AVL tree implementation
- a hashmap implementation
- a generic LRU cache

The library is tested on Linux and should work
on other systems as well. The tests use features
only available on Linux (e.g. high resolution timers)
and won't work on some systems.
The library itself does not require additional libraries;
some tests, however, have external dependencies
(e.g. Google's City Hash algorithm).

The code is licensed under the LGPL V3 with the exception
that static linking is explicitly allowed 
("static linking exception").

The library comes with a GNU Makefile.

- `make`
  builds the library and tests

- `make run`
  runs the tests

- `make all`
   builds the library, tests and tools

- `make debug`
  like `make all` but in debug mode

- `make tools`
   builds the library and tools

- `make lib`
  builds the library only

- `make clean`
  removes all binaries and object files

- `. setenv.sh`
  adds `./lib` to the `LD_LIBRARY_PATH`
