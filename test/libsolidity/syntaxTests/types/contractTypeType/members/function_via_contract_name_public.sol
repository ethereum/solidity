contract A {
    function f() public {}
}

contract B {
    function g() external pure returns(bytes4) {
        return A.f.selector;
    }
}
