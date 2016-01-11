function add -> int {
    var int a;
    asm iadd 0 (arg 1 0) (arg 2 1);
    return a;
}

function print -> void { asm print (arg 1 0); }

function main -> int {
    {
        var int a = 20;
        var int b = 22;
        var int c;
    }
    c = add(a, b);
    print(c);
    return 0;
}
