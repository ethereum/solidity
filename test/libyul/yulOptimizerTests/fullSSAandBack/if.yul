{
  let a := 1
  if calldataload(42) {
    a := 2
  }
  sstore(0, a)
}
// ----
// step: fullSSAandBack
//
// {
//     let a_3 := 1
//     if calldataload(42) { a_3 := 2 }
//     sstore(0, a_3)
// }
