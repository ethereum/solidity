{
  sstore(0, staticcall(gas(), address(), 0, 0x20, 0x20, 0x20))
}
// ====
// EVMVersion: >=byzantium
// ----
// Trace:
//   INVALID()
// Memory dump:
//      0: 0000000000000000000000000000000011111111000000000000000000000000
// Storage dump:
