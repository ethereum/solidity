interface I {
    function f() external;
}

contract B {
    function g() external pure {
        I.f.selector;
    }
}
