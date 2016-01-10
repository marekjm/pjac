begin function foo
    return;
end

begin function bar
    foo();
    return 4;
end
