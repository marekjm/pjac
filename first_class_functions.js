function print(auto m) { asm pring m; }

function main() -> int {
    var auto i = 0;
    var auto p = print;
    var auto x = p;
    x(i);
    return 0;
}
