{
  let a := add(7, sub(mload(0), 7))
  mstore(20, a)
}
// ----
// step: expressionSimplifier
//
// { mstore(20, mload(0)) }
