{
  let a := add(1, mul(3, 4))
  sstore(7, a)
}
// ----
// step: expressionSimplifier
//
// { sstore(7, 13) }
