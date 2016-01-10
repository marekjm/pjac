begin class Foo
    begin function foo
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
    end
end

begin function main
    //var Foo a;
    //a.foo();
end
