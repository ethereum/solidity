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
// TypeError: (114-120): Storage variables cannot be assigned to.
// TypeError: (138-146): Storage variables cannot be assigned to.
