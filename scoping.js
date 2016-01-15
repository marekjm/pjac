function echo(undefined msg) { asm echo msg; }
function print(undefined msg) { asm print msg; }

function main() -> int {
    var string msg = "Oh noes! A bug!";
    var bool i = true;

    if i {
        // defined in local scope of this if-statement
        var int x;
    }

    // compiler accepts this since direct register accesses are unchecked
    // but it will throw an exception at runtime if the condition in
    // the "if" above is false
    asm print 3;

    // this does not pass the compiler since x is not declared in this scope
    print(x);

    return 0;
}
