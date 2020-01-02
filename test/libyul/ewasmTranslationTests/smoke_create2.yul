{
  let size := codesize()
  codecopy(0x40, 0, size)
  sstore(0, create2(42, 0, size, 0x20))
}
// ====
// EVMVersion: >=constantinople
// ----
// Trace:
//   INVALID()
// Memory dump:
//     80: 636f6465636f6465636f6465636f6465636f6465000000000000000000000000
// Storage dump:
