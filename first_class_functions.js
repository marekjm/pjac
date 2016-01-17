function print(auto m) { asm print m; }

function gp() -> auto {
    var auto p = print;
    return p;
}

function main() -> int {
    var auto i = 0;
    var auto p = print;
    //var auto x = p;
    p(i);

    return 0;
}
