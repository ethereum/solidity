contract C {
    fallback(bytes calldata _input) external virtual returns (bytes memory _output) {}
}
contract D is C {
    fallback() external override {}
}
// ----
