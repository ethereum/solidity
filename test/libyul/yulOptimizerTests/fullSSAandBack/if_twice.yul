{
  let a := 1
  if calldataload(42) {
    a := 2
  }
  if calldataload(a) {
    a := add(a, 4)
  }
  sstore(0, a)
}
// ----
// step: fullSSAandBack
//
// {
//     let a_5 := 1
//     if calldataload(42) { a_5 := 2 }
//     let a_2_6 := a_5
//     if calldataload(a_5) { a_2_6 := add(a_5, 4) }
//     sstore(0, a_2_6)
// }
