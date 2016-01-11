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
function print -> void { asm print (arg 1 0); }

function main -> int {
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
    end
.end
```
