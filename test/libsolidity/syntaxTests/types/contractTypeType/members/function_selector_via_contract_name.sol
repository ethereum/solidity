contract A {
    function f() external {}
}

contract B {
    function g() external pure returns(bytes4) {
        return A.f.selector;
    }
}
// ----
