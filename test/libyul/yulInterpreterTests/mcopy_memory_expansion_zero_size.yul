{
    // Should expand memory to two full words (0x40 bytes)
    mcopy(0x30, 0, 1)
    sstore(0, msize())
}
// ====
// EVMVersion: >=cancun
// ----
// Trace:
//   MCOPY(48, 0, 1)
// Memory dump:
// Storage dump:
//   0000000000000000000000000000000000000000000000000000000000000000: 0000000000000000000000000000000000000000000000000000000000000040
