{
  let a := calldataload(42)
  for {} a { sstore(1, a) } {
    a := sub(a,1)
    if lt(a,4) { continue }
    if eq(a,42) { break }
  }
  sstore(0, a)
}
// ----
// step: fullSSAandBack
//
// {
//     let a_5 := calldataload(42)
//     for { } a_5 { sstore(1, a_5) }
//     {
//         let a_2 := sub(a_5, 1)
//         if lt(a_2, 4)
//         {
//             a_5 := a_2
//             continue
//         }
//         if eq(a_2, 42)
//         {
//             a_5 := a_2
//             break
//         }
//         a_5 := a_2
//     }
//     sstore(0, a_5)
// }
