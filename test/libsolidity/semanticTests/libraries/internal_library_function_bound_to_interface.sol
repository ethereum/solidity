interface I {}
contract E is I {}

library L {
    function foo(I i) internal pure returns (uint) {
        return 42;
    }
}

contract C {
    using L for I;

    function test() public returns (uint) {
        E e = new E();
        return I(e).foo();
    }
}

// ----
// test() -> 42
