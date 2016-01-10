begin function print
    asm print (arg 1 0);
    return;
end

begin function main
    var string msg = "Hello World!";
    print(msg);
    return 0;
end
