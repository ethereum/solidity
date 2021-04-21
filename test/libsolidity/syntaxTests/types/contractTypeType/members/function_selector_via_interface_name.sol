interface I {
    function f() external;
}

contract B {
    function g() external pure returns(bytes4) {
        return I.f.selector;
    }
}
// ----
