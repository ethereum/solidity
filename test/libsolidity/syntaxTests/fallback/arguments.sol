contract C {
    fallback(uint256) external {}
}
// ----
// TypeError: (25-34): Fallback function cannot take parameters.
