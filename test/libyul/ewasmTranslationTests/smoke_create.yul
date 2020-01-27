{
  let size := codesize()
  codecopy(0, 0, size)
  sstore(0, create(42, 0, size))
}
// ----
// Trace:
//   CREATE()
// Memory dump:
//     40: 636f6465636f6465636f6465636f6465636f6465000000000000000000000000
// Storage dump:
