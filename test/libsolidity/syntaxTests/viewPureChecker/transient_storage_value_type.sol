contract C {
    uint transient x;

    function f() public view {
        x = 1;
    }
    function g() public pure returns (uint a) {
        a = x + 1;
    }
}
// ====
// EVMVersion: >=cancun
// ----
// TypeError 8961: (75-76): Function cannot be declared as view because this expression (potentially) modifies the state.
// TypeError 2527: (148-149): Function declared as pure, but this expression (potentially) reads from the environment or state and thus requires "view".
