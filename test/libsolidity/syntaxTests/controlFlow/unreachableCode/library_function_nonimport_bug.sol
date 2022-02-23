contract Bar {
    function example() public {
        foo();
        return;
    }

    function foo() internal {
        Foo.nop();
    }
}

contract Y is Bar {}

library Foo {
    function nop() internal {}
}
