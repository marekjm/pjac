function print(undefined msg) { asm print msg; }
function boolean(undefined o) -> bool { asm not (not o); var bool b = o; return b; }

function main() -> int {
    var string msg = "Hello World!";
    print(msg);
    var bool f;
    f = boolean(msg);
    print(f);

    return 0;
}
