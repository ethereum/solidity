contract C {
    fallback() external returns (bytes memory, bytes memory) {}
}
// ----
// TypeError 5570: (45-73): Fallback function either has to have the signature "fallback()" or "fallback(bytes calldata) returns (bytes memory)".
