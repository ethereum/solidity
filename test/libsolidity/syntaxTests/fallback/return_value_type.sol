contract C {
    fallback() external returns (uint256) {}
}
// ----
// TypeError 5570: (45-54): Fallback function either has to have the signature "fallback()" or "fallback(bytes calldata) returns (bytes memory)".
