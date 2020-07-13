contract C {
    uint[] x;
    fallback() external {
        uint y_slot = 2;
        uint y_offset = 3;
        uint[] storage y = x;
        assembly {
            pop(y_slot)
            pop(y_offset)
        }
        y[0] = 2;
    }
}
// ----
