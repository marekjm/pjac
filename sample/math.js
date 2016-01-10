begin function add
    var int a;
    asm iadd 0 (arg 1 0) (arg 2 1);
    return a;
end

begin function print
    asm print (arg 1 0);
    return;
end

begin function main
    var int a = 20;
    var int b = 22;
    var int c;
    c = add(a, b);
    print(c);
    return 0;
end
