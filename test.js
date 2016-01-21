function echo(auto msg) { asm echo msg; }
function print(auto msg) { asm print msg; }
function decrement(int a) -> int { asm idec a; return a; }

function main()->int{
    var int i = 10;
    var string msg = "iterations left: ";
    var bool stop;
    var int stopper = 4;

    while i {
        i = decrement(i);
        print(i);
    }

    return 0;
}
