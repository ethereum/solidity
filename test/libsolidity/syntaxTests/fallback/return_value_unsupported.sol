contract C {
    fallback() external returns (bytes memory) {}
}
// ----
// TypeError 5570: (45-59='(bytes memory)'): Fallback function either has to have the signature "fallback()" or "fallback(bytes calldata) returns (bytes memory)".
