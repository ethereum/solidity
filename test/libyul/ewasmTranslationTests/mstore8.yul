{
  mstore(0x30, not(0))
  mstore8(0x20, 0xaa)
  mstore8(0x25, 0xbbbb)
  mstore8(0x26, 0xcc)
  mstore8(0x3b, 0x11)
  sstore(0, mload(0x20))
}
// ----
// Trace:
//   INVALID()
// Memory dump:
//     60: 00000000000000000000000000000000ffffffffffffffffffffffffffffffff
//     80: ffffffffffffffffffffffffffffffff00000000000000000000000000000000
// Storage dump:
