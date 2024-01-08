{
    // Should not affect msize
    mcopy(0x30, 0x30, 0)
    sstore(0, msize())
}
// ====
// EVMVersion: >=cancun
// ----
// Trace:
//   MCOPY(48, 48, 0)
// Memory dump:
// Storage dump:
