function print(auto m) { asm print m; }

function assign(auto object) -> auto { return object; }

function main() -> int {
    var string hello = "Hello World!";
    var string bye = "Bye World!";

    var string msg;

    msg = assign(hello);
    print(msg);

    msg = assign(bye);
    print(msg);

    return 0;
}
