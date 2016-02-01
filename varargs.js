function echo_one(auto msg) { asm echo msg; }
function echo(auto msg...) { asm echo msg; }

function print_one(auto msg) { asm print msg; }
function print(auto msg...) { asm print msg; }

function main() -> int {
    print("Hello World!");
    return 0;
}
