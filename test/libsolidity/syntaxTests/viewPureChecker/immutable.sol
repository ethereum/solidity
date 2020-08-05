contract B {
    uint immutable x = 1;
    function f() public pure returns (uint) {
        return x;
    }
}
// ----
// TypeError 2527: (100-101): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
