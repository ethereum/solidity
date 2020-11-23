contract C {
    fallback(uint256) external {}
}
// ----
// TypeError 5570: (44-44): Fallback function either has to have the signature "fallback()" or "fallback(bytes calldata) returns (bytes memory)".
