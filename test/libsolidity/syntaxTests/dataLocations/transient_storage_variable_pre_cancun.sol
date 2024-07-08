contract C {
    uint transient x;
}
// ====
// EVMVersion: <cancun
// ----
// DeclarationError 7985: (17-33): Transient storage is not supported by EVM versions older than cancun.
