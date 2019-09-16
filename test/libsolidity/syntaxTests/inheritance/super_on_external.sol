contract A {
    function f() external pure {}
}
contract B is A {
    function f() public override pure {
        super.f();
    }
}
// ----
// TypeError: (115-122): Member "f" not found or not visible after argument-dependent lookup in contract super B.
