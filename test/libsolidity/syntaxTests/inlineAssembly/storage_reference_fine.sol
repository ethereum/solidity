contract C {
    uint[] x;
    fallback() external {
        uint[] storage y = x;
        assembly {
            pop(y_slot)
            pop(y_offset)
        }
    }
}
// ----
