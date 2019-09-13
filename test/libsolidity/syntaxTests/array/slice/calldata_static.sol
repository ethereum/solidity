contract C {
    function f(uint256[42] calldata x) external pure {
        x[1:2];
    }
}
// ----
// TypeError: (76-82): Index range access is only supported for dynamic calldata arrays.
