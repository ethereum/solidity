{
  let a := 1
  a := 2
  sstore(0, a)
}
// ----
// step: fullSSATransform
//
// {
//     let a := 1
//     let a_1 := 2
//     sstore(0, a_1)
// }
