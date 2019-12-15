{
  sstore(0, callcode(gas(), address(), 42, 0, 0x20, 0x20, 0x20))
}
// ----
// Trace:
//   INVALID()
// Memory dump:
//      0: 0000000000000000000000000000000011111111000000000000000000000000
// Storage dump:
