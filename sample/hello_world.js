function print -> void {
    asm print (arg 1 0);
    return;
}

function main -> int {
    var string msg = "Hello World!";
    print(msg);
    return 0;
}
