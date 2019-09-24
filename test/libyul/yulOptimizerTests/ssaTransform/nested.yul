{
  let a := 1
  a := 2
  let b := 3
  b := 4
  {
    // b is not reassigned here
    a := 3
    a := 4
  }
  a := add(b, a)
}
// ====
// step: ssaTransform
// ----
// {
//     let a_1 := 1
//     let a := a_1
//     let a_2 := 2
//     a := a_2
//     let b_3 := 3
//     let b := b_3
//     let b_4 := 4
//     b := b_4
//     {
//         let a_5 := 3
//         a := a_5
//         let a_6 := 4
//         a := a_6
//     }
//     let a_8 := a
//     let a_7 := add(b_4, a_8)
//     a := a_7
// }
