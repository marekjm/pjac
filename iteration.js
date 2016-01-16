function echo(auto msg) { asm echo msg; }
function print(auto msg) { asm print msg; }
function decrement(int a) -> int { asm idec a; return a; }
function integerEquals(int a, int b) -> bool {
    var bool result;
    asm ieq result a b;
    return result;
}

function main() -> int {
    var int i = 10;
    var string msg = "iterations left: ";
    var bool stop;
    var int stopper = 4;

    while i {
        stop = integerEquals(i, stopper);
        if stop {
            var string stop_msg = "breaking!";
            print(stop_msg);
            break;
        }
        echo(msg);
        i = decrement(i);
        print(i);
    }

    return 0;
}
