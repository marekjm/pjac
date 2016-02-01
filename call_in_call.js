function echo(auto msg) { asm echo msg; }
function print(auto msg) { asm print msg; }

function decrement(int i) -> int {
    asm idec i;
    return i;
}
function increment(int i) -> int {
    asm iinc i;
    return i;
}

function add(int a, int b) -> int {
    asm iadd a a b;
    return a;
}
function multiply(int a, int b) -> int {
    asm imul a a b;
    return a;
}

function equalsInt(int a, int b) -> bool {
    var bool eq;
    asm ieq eq a b;
    return eq;
}

function pow(int n, int to) -> int {
    var bool stop = false;
    stop = equalsInt(0, to);
    if stop {
        return 1;
    }

    n = multiply(n, pow(n, decrement(to)));
    return n;
}

function main() -> int {
    print(add(multiply(3, 7), multiply(3, 7)));

    echo("pow(2, 3) => ");
    print(pow(2, 3));

    return 0;
}
