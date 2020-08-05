contract C {
    function f(uint256[] calldata x) external pure {
        abi.encode(x[1:2]);
    }
}
// ----
