function echo(auto msg) { asm echo msg; }
function print(auto msg) { asm print msg; }

function main() -> int {
    print("Hello World!");
    return 0;
}
