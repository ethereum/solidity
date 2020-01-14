contract A {
    function f() public {}
}

contract B {
    function g() external pure {
        A.f.selector;
    }
}
