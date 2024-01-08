{
    mstore8(0xffffffffffffffff, 1)

    // Interpreter ignores memory copies with very large offset and/or size.
    // None of these will show up in memory dump.
    mcopy(0, 0xffffffffffffffff, 1)
    mcopy(0xffffffffffffffff, 0, 1)
    mcopy(0, 1, 0xffffffffffffffff)
    mcopy(0xffffffff00000000, 0xffffffffffffffff, 0xffffffffffffffff)
}
// ====
// EVMVersion: >=cancun
// ----
// Trace:
//   MCOPY(0, 0xffffffffffffffff, 1)
//   MCOPY(0xffffffffffffffff, 0, 1)
//   MCOPY(0, 1, 0xffffffffffffffff)
//   MCOPY(0xffffffff00000000, 0xffffffffffffffff, 0xffffffffffffffff)
// Memory dump:
//   FFFFFFFFFFFFFFE0: 0000000000000000000000000000000000000000000000000000000000000001
// Storage dump:
