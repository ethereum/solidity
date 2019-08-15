contract C {
    function f(uint256[] calldata x) external pure {
        x[0] = 42;
    }
}
// ----
// TypeError: (74-78): Calldata arrays are read-only.
