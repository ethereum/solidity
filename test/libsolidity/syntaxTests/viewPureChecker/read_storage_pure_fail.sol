contract C {
    uint x;
    function f() public pure returns (uint) {
        return x;
    }
}
// ----
// TypeError: (86-87): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
