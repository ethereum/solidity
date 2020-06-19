contract C {
    fallback() external returns (uint) { }
}
// ----
// TypeError 5570: (45-51): Fallback function can only have a single "bytes memory" return value.
