{
  let a := calldataload(42)
  for {} a { for {} a { a := sub(a, 1) } { sstore(1, a) }} {
    a := sub(a,1)
  }
  sstore(0, a)
}
// ----
// step: fullSSAandBack
//
// {
//     let a_9 := calldataload(42)
//     for { }
//     a_9
//     {
//         let a_3_10 := a_9
//         for { } a_3_10 { a_3_10 := sub(a_3_10, 1) }
//         { sstore(1, a_3_10) }
//         a_9 := a_3_10
//     }
//     { a_9 := sub(a_9, 1) }
//     sstore(0, a_9)
// }
