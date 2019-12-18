{
  sstore(0, smod(0, 1))
  sstore(1, smod(1, not(0)))
  sstore(2, smod(0, 0))
  sstore(3, smod(1, 2))
  sstore(4, smod(not(0), 1))
  sstore(5, smod(
    0x8000000000000000000000000000000000000000000000000000000000000000, 1
  ))
  sstore(6, smod(not(0), 1))
  sstore(7, smod(0xffffffffffffffffffffffffffffffff, 1))
  sstore(8, smod(0xffffffffffffffffffffffffffffffffffffffffffffffff, 0xfffffffffffffffffffffffffffffffe))
  sstore(9, smod(0xffffffffffffffffffffffffffffffffffffffffffffffff, 5))
  sstore(10, smod(0xffffffffffffffffffffffffffffffffffffffffffffffff, 4))
  sstore(11, smod(0xffffffffffffffffffffffffffffffff, 3))
  sstore(12, smod(0xffffffffffffffff, 3))
  sstore(13, smod(0xffffffffffffffffffffffffffffffff0000000000000000, 0xffffffffffffffff43342553000))
  sstore(14, smod(
    0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffd,
    0xffffffffffffffff43342553000
  ))
}
// ----
// Trace:
// Memory dump:
//      0: 0000000000000000000000000000000000000000000000000000000000000001
//     20: 0000000000000000000000000000000000000000000000000000000000000001
// Storage dump:
//   0000000000000000000000000000000000000000000000000000000000000001: 0000000000000000000000000000000000000000000000000000000000000001
