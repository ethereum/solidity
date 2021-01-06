{
  let a := 0
  for {} lt(a, 10) {
    // checked increment
    if eq(a, sub(0, 1)) { revert(0, 0) }
    a := add(a, 1)
  } {
    sstore(a, 42)
  }
}
// ----
// step: fullSSATransform
//
// {
//     let a := 0
//     phi_store("a", a)
//     for { }
//     true
//     {
//         let a_2 := phi_load("a")
//         if eq(a_2, sub(0, 1)) { revert(0, 0) }
//         let a_3 := add(a_2, 1)
//         phi_store("a", a_3)
//     }
//     {
//         let a_1 := phi_load("a")
//         if iszero(lt(a_1, 10))
//         {
//             phi_store("a", a_1)
//             break
//         }
//         sstore(a_1, 42)
//         phi_store("a", a_1)
//     }
//     let a_4 := phi_load("a")
// }
