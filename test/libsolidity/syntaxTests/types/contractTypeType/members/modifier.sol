contract A {
    modifier mod() { _; }
}
contract B {
    function f() public {
        A.mod;
    }
}
// ----
// TypeError: (88-93): Member "mod" not found or not visible after argument-dependent lookup in type(contract A).
