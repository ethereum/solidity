contract C {
    function f(uint256[] calldata x) external pure {
        x[:][:10];
    }
}
// ----
