contract C {
    fallback() external returns (bytes memory _output) {}
}
contract D {
    fallback(bytes calldata _input) external {}
}
// ----
// TypeError 5570: (45-67): Fallback function either has to have the signature "fallback()" or "fallback(bytes calldata) returns (bytes memory)".
// TypeError 5570: (131-131): Fallback function either has to have the signature "fallback()" or "fallback(bytes calldata) returns (bytes memory)".
