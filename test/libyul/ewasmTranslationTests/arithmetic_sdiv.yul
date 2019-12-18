{
  sstore(0, sdiv(0, 1))
  sstore(1, sdiv(0, not(0)))
  sstore(2, sdiv(0, 0))
  sstore(3, sdiv(1, 2))
  sstore(4, sdiv(not(0), 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe))
  sstore(5, sdiv(0x8000000000000000000000000000000000000000000000000000000000000000, not(0)))
  sstore(6, sdiv(not(0), 0x8000000000000000000000000000000000000000000000000000000000000000))
  sstore(7, sdiv(0x7000000000000000000000000000000000000000000000000000000000000000, 1))
  sstore(8, sdiv(1, 0x7000000000000000000000000000000000000000000000000000000000000000))
  sstore(9, sdiv(0x7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, not(0)))
  sstore(10, sdiv(not(0), 0x7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff))
  sstore(11, sdiv(0x7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff, 1))
  sstore(12, sdiv(not(0), not(0)))
  sstore(13, sdiv(
    0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe,
    0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe
  ))
  sstore(14, sdiv(0x8000000000000000000000000000000000000000000000000000000000000001, not(0)))
}
// ----
// Trace:
//   INVALID()
// Memory dump:
// Storage dump:
