function echo(auto msg) { asm echo msg; }
function print(auto msg) { asm print msg; }
function decrement(int a) -> int { asm idec a; return a; }

function recursive(int counter) {
    var string msg = "counter value is: ";
    echo(msg);
    print(counter);
    counter = decrement(counter);
    if counter {
        recursive(counter);
    }
}

function main() -> int {
    var string a = "Woohoo, recursion!";
    print(a);

    var int limit = 10;
    recursive(limit);

    return 0;
}
