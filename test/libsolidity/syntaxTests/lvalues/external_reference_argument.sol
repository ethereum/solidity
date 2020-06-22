contract C {
    function f(uint256[] calldata x, uint256[] calldata y) external pure {
        x = y;
    }
}
// ----
