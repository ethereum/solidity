{
  let a := calldataload(42)
  for {} a { sstore(1, a) } {
    a := sub(a,1)
  }
  sstore(0, a)
}
// ----
// step: fullSSAandBack
//
// {
//     let a_5 := calldataload(42)
//     for { } a_5 { sstore(1, a_5) }
//     { a_5 := sub(a_5, 1) }
//     sstore(0, a_5)
// }
