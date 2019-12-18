{
  sstore(0, addmod(0, 1, 1))
  sstore(1, addmod(0, 1, 2))
  sstore(2, addmod(3, 1, 2))
  sstore(3, addmod(1, not(0), 5))
  sstore(4, addmod(0, 0, 0))
  sstore(5, addmod(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 1, 1))
  sstore(6, addmod(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe, 1, 0))
  sstore(7, addmod(0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffd, 1, 5))
  sstore(8, addmod(
    0xffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff,
    0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffe,
    5
  ))
}
// ----
// Trace:
//   INVALID()
// Memory dump:
// Storage dump:
