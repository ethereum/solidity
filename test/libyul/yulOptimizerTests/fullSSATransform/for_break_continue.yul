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
//         if lt(a_2, 4)
//         {
//             phi_store("a", a_2)
//             continue
//         }
//         if eq(a_2, 42)
//         {
//             phi_store("a", a_2)
//             break
//         }
//         phi_store("a", a_2)
//     }
//     let a_4 := phi_load("a")
//     sstore(0, a_4)
// }
