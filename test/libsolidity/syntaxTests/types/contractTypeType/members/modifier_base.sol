contract A {
    modifier mod() { _; }
}
contract B is A {
    function f() public {
        A.mod;
    }
}
// ----
// TypeError 9582: (93-98): Member "mod" not found or not visible after argument-dependent lookup in type(contract A).
