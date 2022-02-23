contract C {
    uint[] x;
    fallback() external {
        uint[] storage y = x;
        assembly {
            y.slot := 1
            y.offset := 2
        }
    }
}
// ----
// TypeError 9739: (138-146): Only .slot can be assigned to.
