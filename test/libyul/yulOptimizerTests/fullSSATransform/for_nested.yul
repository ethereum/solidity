{
  let a := calldataload(42)
  for {} a { for {} a { a := sub(a, 1) } { sstore(1, a) }} {
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
//         phi_store("a_3", a_3)
//         for { }
//         phi_load("a_3")
//         {
//             let a_5 := phi_load("a_3")
//             let a_6 := sub(a_5, 1)
//             phi_store("a_3", a_6)
//         }
//         {
//             let a_4 := phi_load("a_3")
//             sstore(1, a_4)
//             phi_store("a_3", a_4)
//         }
//         let a_7 := phi_load("a_3")
//         phi_store("a", a_7)
//     }
//     {
//         let a_1 := phi_load("a")
//         let a_2 := sub(a_1, 1)
//         phi_store("a", a_2)
//     }
//     let a_8 := phi_load("a")
//     sstore(0, a_8)
// }
