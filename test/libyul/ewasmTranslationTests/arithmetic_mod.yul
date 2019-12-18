{
  sstore(0, mod(0, 1))
  sstore(1, mod(1, not(0)))
  sstore(2, mod(0, 0))
  sstore(3, mod(1, 2))
  sstore(4, mod(not(0), 1))
  sstore(5, mod(
    0x8000000000000000000000000000000000000000000000000000000000000000, 1
  ))
  sstore(6, mod(not(0), 1))
  sstore(7, mod(0xffffffffffffffffffffffffffffffff, 1))
  sstore(8, mod(0xffffffffffffffffffffffffffffffffffffffffffffffff, 0xfffffffffffffffffffffffffffffffe))
  sstore(9, mod(0xffffffffffffffffffffffffffffffffffffffffffffffff, 5))
  sstore(10, mod(0xffffffffffffffffffffffffffffffffffffffffffffffff, 4))
  sstore(11, mod(0xffffffffffffffffffffffffffffffff, 3))
  sstore(12, mod(0xffffffffffffffff, 3))
  sstore(13, mod(0xffffffffffffffffffffffffffffffff0000000000000000, 0xffffffffffffffff43342553000))
}
// ----
// Trace:
// Memory dump:
//      0: 0000000000000000000000000000000000000000000000000000000000000001
//     20: 0000000000000000000000000000000000000000000000000000000000000001
// Storage dump:
//   0000000000000000000000000000000000000000000000000000000000000001: 0000000000000000000000000000000000000000000000000000000000000001
