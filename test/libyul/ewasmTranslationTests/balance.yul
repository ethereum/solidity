{
  sstore(0, balance(address()))
  sstore(1, balance(0))
}
// ----
// Trace:
//   INVALID()
// Memory dump:
//      0: 0000000000000000000000000000000011111111000000000000000000000000
// Storage dump:
