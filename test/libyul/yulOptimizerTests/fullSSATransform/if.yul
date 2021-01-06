{
  let a := 1
  if calldataload(42) {
    a := 2
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
//     sstore(0, a_2)
// }
