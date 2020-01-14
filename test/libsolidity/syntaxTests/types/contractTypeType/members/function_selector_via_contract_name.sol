contract A {
    function f() external {}
}

contract B {
    function g() external pure {
        A.f.selector;
    }
}
