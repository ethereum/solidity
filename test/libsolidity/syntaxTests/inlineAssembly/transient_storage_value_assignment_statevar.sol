contract C {
    uint transient x;
    fallback() external {
        assembly {
            x.slot := 1
            x.offset := 2
        }
    }
}
// ====
// EVMVersion: >=cancun
// ----
// TypeError 4713: (92-98): State variables cannot be assigned to - you have to use "sstore()" or "tstore()".
// TypeError 4713: (116-124): State variables cannot be assigned to - you have to use "sstore()" or "tstore()".
