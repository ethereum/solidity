{
  let a := 1
  switch a
  case 1
  {
    a := calldataload(1)
  }
  case 2
  {
    a := calldataload(a)
  }
  default
  {
  }
  sstore(0, a)
}
// ----
// step: fullSSATransform
//
// {
//     let a := 1
//     phi_store("a", a)
//     switch a
//     case 1 {
//         let a_1 := calldataload(1)
//         phi_store("a", a_1)
//     }
//     case 2 {
//         let a_2 := calldataload(a)
//         phi_store("a", a_2)
//     }
//     default { phi_store("a", a) }
//     let a_3 := phi_load("a")
//     sstore(0, a_3)
// }
