contract C {
    fallback() external returns (uint) { }
}
// ----
// TypeError 5570: (45-51): Fallback function either has to have the signature "fallback()" or "fallback(bytes calldata) returns (bytes memory)".
