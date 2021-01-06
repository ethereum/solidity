{
  let a := calldataload(0)
  a := calldataload(1)
  sstore(0, a)
}
// ----
// step: fullSSAandBack
//
// { sstore(0, calldataload(1)) }
