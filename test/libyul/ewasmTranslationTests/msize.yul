{
  sstore(0, msize())
  mstore(0x20, 0x0102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f20)
  mstore(0x40, mload(0x20))
  sstore(1, mload(0x40))
  sstore(2, msize())
}
// ----
// Trace:
//   INVALID()
// Memory dump:
// Storage dump:
