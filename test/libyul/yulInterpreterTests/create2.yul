{
    pop(create2(0, 0, 32, 32))
    // This used to store 64
    sstore(0, msize())
}
// ====
// EVMVersion: >=constantinople
// ----
// Trace:
//   CREATE2(0, 0, 32, 32)
// Memory dump:
// Storage dump:
//   0000000000000000000000000000000000000000000000000000000000000000: 0000000000000000000000000000000000000000000000000000000000000020
