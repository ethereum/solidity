contract C {
    fallback(uint256) external {}
}
// ----
// TypeError 3978: (25-34): Fallback function cannot take parameters.
