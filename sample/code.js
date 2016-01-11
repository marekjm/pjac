begin class Foo
    function foo -> void {
        var int a = 42;
        var float pi = 3.14;
        var string msg = "Hello World!";
        var string msg_singlequoted = 'Hello "Worldy" World!';

        /* A block
         * comment.
         */

        var float c;

        // automatic type conversion for `a` from `int` to `float`
        // c = 45.14
        //c = a + b;

        print(a);
        print(b);
        print(c);
        print(msg);
    }
end

function main -> int {
    //var Foo a;
    //a.foo();
    return 0;
}
