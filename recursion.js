function print(undefined msg) {
    var string a = "Bye World!";
    asm print msg;
}

function returnTheIntegerIGaveYou(int a) -> int { return a; }

function main() -> int {
    var string a = "Hello World!";
    print(a);

    var int answer = 42;
    var int i;
    i = returnTheIntegerIGaveYou(answer);
    print(i);

    if;

    return 0;
}
