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
// step: fullSSATransform
//
// {
//     let a := 1
//     phi_store("a", a)
//     if calldataload(42)
//     {
//         let a_1 := 2
//         phi_store("a", a_1)
//     }
//     let a_2 := phi_load("a")
//     phi_store("a_2", a_2)
//     if calldataload(a_2)
//     {
//         let a_3 := add(a_2, 4)
//         phi_store("a_2", a_3)
//     }
//     let a_4 := phi_load("a_2")
//     sstore(0, a_4)
// }
