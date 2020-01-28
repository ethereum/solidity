{
  sstore(0, mulmod(0, 1, 2))
  sstore(1, mulmod(1, not(0), 5))
  sstore(2, mulmod(0, 0, 5))
  sstore(3, mulmod(1, 3, 2))
  sstore(4, mulmod(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe,
    1,
    0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff))
  sstore(5, mulmod(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 1, 1))
  sstore(6, mulmod(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 1, 0))
  sstore(7, mulmod(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffd, 1, 5))
  sstore(8, mulmod(
    not(0),
    0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe,
    25
  ))
}
// ----
// Trace:
// Memory dump:
//      0: 0000000000000000000000000000000000000000000000000000000000000008
//     20: 000000000000000000000000000000000000000000000000000000000000000f
// Storage dump:
//   0000000000000000000000000000000000000000000000000000000000000003: 0000000000000000000000000000000000000000000000000000000000000001
//   0000000000000000000000000000000000000000000000000000000000000004: fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe
//   0000000000000000000000000000000000000000000000000000000000000007: 0000000000000000000000000000000000000000000000000000000000000003
//   0000000000000000000000000000000000000000000000000000000000000008: 000000000000000000000000000000000000000000000000000000000000000f
