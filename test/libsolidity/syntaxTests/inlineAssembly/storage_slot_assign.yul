contract C {
    uint[] x;
    fallback() external {
        uint[] storage y = x;
        assembly {
            y_slot := 1
            y_offset := 2
        }
    }
}
// ----
// TypeError: (138-146): Only _slot can be assigned to.
