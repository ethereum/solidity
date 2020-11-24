contract C {
    uint x;
    fallback(uint a) external { x = 2; }
}
// ----
// TypeError 5570: (55-55): Fallback function either has to have the signature "fallback()" or "fallback(bytes calldata) returns (bytes memory)".
