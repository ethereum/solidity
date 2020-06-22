contract C {
    fallback() external returns (uint256) {}
}
// ----
// TypeError 5570: (45-54): Fallback function can only have a single "bytes memory" return value.
