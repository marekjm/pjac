function echo(undefined msg) { asm echo msg; }
function print(undefined msg) { asm print msg; }

function main() -> int {
    var string msg = "Hello World!";
    var bool i = true;

    if i {
        // defined in local scope of this if-statement
        var string msg = "Oh noes! A bug!";

        {
            var bool f;
        }
        // uncomment this to make compilation fail
        //print(f);

        if msg {
            // should print "Oh noes! A bug!"
            print(msg);
        }
    }

    if msg {
        // should print "Hello World!"
        print(msg);
    }

    return 0;
}
