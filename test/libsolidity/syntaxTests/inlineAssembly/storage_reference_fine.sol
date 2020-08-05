contract C {
    uint[] x;
    fallback() external {
        uint[] storage y = x;
        assembly {
            pop(y.slot)
            pop(y.offset)
        }
    }
}
// ----
