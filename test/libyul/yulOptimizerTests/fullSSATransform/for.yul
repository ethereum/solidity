{
  let a := calldataload(42)
  for {} a { sstore(1, a) } {
    a := sub(a,1)
  }
  sstore(0, a)
}
// ----
// step: fullSSATransform
//
// {
//     let a := calldataload(42)
//     phi_store("a", a)
//     for { }
//     phi_load("a")
//     {
//         let a_3 := phi_load("a")
//         sstore(1, a_3)
//         phi_store("a", a_3)
//     }
//     {
//         let a_1 := phi_load("a")
//         let a_2 := sub(a_1, 1)
//         phi_store("a", a_2)
//     }
//     let a_4 := phi_load("a")
//     sstore(0, a_4)
// }
