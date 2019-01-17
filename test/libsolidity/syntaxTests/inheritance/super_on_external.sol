contract A {
    function f() external pure {}
}
contract B is A {
    function f() public pure {
        super.f();
    }
}
// ----
// TypeError: (106-113): Member "f" not found or not visible after argument-dependent lookup in contract super B.
