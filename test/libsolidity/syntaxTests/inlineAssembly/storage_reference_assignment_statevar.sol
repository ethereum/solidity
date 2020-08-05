contract C {
    uint[] x;
    fallback() external {
        assembly {
            x.slot := 1
            x.offset := 2
        }
    }
}
// ----
// TypeError 4713: (84-90): State variables cannot be assigned to - you have to use "sstore()".
// TypeError 4713: (108-116): State variables cannot be assigned to - you have to use "sstore()".
