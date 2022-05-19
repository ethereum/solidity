contract E {}

library L {
    function foo(E e) internal pure returns (uint) {
        return 42;
    }
}

contract C {
    using L for E;

    function test() public returns (uint) {
        E e = new E();
        return e.foo();
    }
}

// ----
// test() -> 42
