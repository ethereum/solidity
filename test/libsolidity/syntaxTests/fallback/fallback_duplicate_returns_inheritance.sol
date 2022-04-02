contract C {
    fallback(bytes calldata _input) external returns (bytes memory _output) {}
}
contract D is C {
    fallback() external {}
}
// ----
// TypeError 9456: (116-138='fallback() external {}'): Overriding function is missing "override" specifier.
// TypeError 4334: (17-91): Trying to override non-virtual function. Did you forget to add "virtual"?
