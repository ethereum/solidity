contract C {
    function f(uint256[] calldata x) external pure {
        abi.encode(x[1:2]);
    }
}
// ----
// TypeError: (85-91): This type cannot be encoded.
