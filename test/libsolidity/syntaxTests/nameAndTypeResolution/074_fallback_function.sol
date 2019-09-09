contract C {
    uint x;
    fallback() external { x = 2; }
}
