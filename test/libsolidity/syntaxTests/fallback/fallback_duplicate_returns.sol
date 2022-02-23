contract C {
    fallback(bytes calldata _input) external returns (bytes memory _output) {}
    fallback() external {}
}
// ----
// DeclarationError 7301: (96-118): Only one fallback function is allowed.
