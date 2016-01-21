# PJAC

PJAC is an acronym from "Polsko-Japońska Akademia Technik Komputerowych" (meaning
"Polish-Japanese Computer Techniques Academy" in Polish) and "compiler".

It is a compiler from a higher-level language to Viua VM assembly written as
first-semester project at PJATK.
And, well, the compiler is hand-hacked, buggy and *very* simple.

----

## The canonical program

Here's "Hello World!" in PJAC-compilable code.

```
// inline assembly to access the "print" and "arg" instructions
function print(auto msg) { asm print msg; }
function main() -> int {
    var string s = "Hello World!";
    print(s);
    return 0;  // only integers can be returned this way
}
```

Surprisingly, it's actually *more* code than the equivalent program
written directly in Viua assembly:

```
.function: main
    print (strstore 1 "Hello World!")
    izero 0
    return
.end
```

However, the longer the program is, the more lines are saved when it is written
in PJAC-compilable language rather than in Viua assembly.


----

## Compilation

Compilation of PJAC requires GCC at least 5.1.
Clang support has not been tested.

The process is automated by Make.
It is sufficient to enter project directory and
type `make` into your console to build PJAC if your
system is properly configured.

PJAC is self-contained.
There are no external dependencies beside the standard C++11 library.


----

## Usage

Use PJAC as described below:

```
./build/bin/pjac <source_code_file>
```

If the file contains valid source code (see attached `.js` files to see examples of valid code),
the program will create `<source_code_file>.asm` file in your working directory.

The resulting file contains the original source compiled into Viua VM assembly language and
is suitable for assembling using `viua-asm` program.
The Viua assembler must be installed separately and
is **not** a part of this project.


#### Assembling and running compiled files

This assumes you have Viua VM installed on your system and
VM's binaries available somewhere in your `$PATH`.

```
$ ./build/bin/pjac <source_file>
$ viua-asm <source_file>.asm
$ viua-vm a.out
Hello World!
```

The output from `viua-vm a.out` will probably not be `Hello World!` but the process of compilation and
assembling stays the same no matter what is found in the source files.
