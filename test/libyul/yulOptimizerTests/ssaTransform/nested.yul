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
// ----
// ssaTransform
// {
//     let a_1 := 1
//     let a := a_1
//     let a_2 := 2
//     a := a_2
//     let b_1 := 3
//     let b := b_1
//     let b_2 := 4
//     b := b_2
//     {
//         let a_3 := 3
//         a := a_3
//         let a_4 := 4
//         a := a_4
//     }
//     let a_5 := add(b_2, a)
//     a := a_5
// }
