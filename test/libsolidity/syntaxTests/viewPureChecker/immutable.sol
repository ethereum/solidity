contract B {
    uint immutable x;
    function f() public pure returns (uint) {
        return x;
    }
}
// ----
// TypeError: (96-97): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
