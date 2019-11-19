contract A {
    function f() external virtual pure {}
}
contract B is A {
    function f() public override pure {
        super.f();
    }
}
// ----
// TypeError: (123-130): Member "f" not found or not visible after argument-dependent lookup in contract super B.
