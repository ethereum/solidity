{
  sstore(0, byte(0, shr(0x9, calldataload(0))))
}
// ====
// EVMVersion: >=constantinople
// ----
// step: expressionSimplifier
//
// { { sstore(0, 0) } }
