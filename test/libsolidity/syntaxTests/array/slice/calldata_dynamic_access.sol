contract C {
    function f(uint256[] calldata x) external pure {
        x[1:2][0];
        x[1:][0];
        x[1:][1:2][0];
        x[1:2][1:][0];
    }
}
// ----
