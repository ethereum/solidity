contract C {
    uint x;
    fallback(uint a) external { x = 2; }
}
// ----
// TypeError: (37-45): Fallback function cannot take parameters.
