function print(auto m) { asm print m; }

function main() -> int {
    var auto s = "Hello automatic World!";
    var auto i = 42;
    var auto b = true;
    print(s);
    print(i);
    print(b);
    return 0;
}
