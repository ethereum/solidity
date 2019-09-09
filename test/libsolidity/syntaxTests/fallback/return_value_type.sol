contract C {
    fallback() external returns (uint256) {}
}
// ----
// TypeError: (45-54): Fallback function can only have a single "bytes memory" return value.
