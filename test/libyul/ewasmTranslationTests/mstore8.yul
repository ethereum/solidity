{
  mstore(0x30, not(0))
  mstore8(0x20, 0xaa)
  mstore8(0x26, 0xcc)
  mstore8(0x3b, 0x11)
  sstore(0, mload(0x20))
}
// ----
// Trace:
// Memory dump:
//     20: aa0000000000cc000000000000000000ffffffffffffffffffffff11ffffffff
//     60: aa0000000000cc000000000000000000ffffffffffffffffffffff11ffffffff
//     80: ffffffffffffffffffffffffffffffff00000000000000000000000000000000
// Storage dump:
//   0000000000000000000000000000000000000000000000000000000000000000: aa0000000000cc000000000000000000ffffffffffffffffffffff11ffffffff
