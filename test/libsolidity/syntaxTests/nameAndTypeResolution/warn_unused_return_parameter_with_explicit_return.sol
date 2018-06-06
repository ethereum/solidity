contract C {
    function f() pure public returns (uint a) {
        return;
    }
}
// ----
// Warning: (51-57): Unused function parameter. Remove or comment out the variable name to silence this warning.
