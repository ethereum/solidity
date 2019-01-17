{
  let a := 1
  a := 2
  a := 3
  a := 4
}
// ----
// ssaTransform
// {
//     let a_1 := 1
//     let a := a_1
//     let a_2 := 2
//     a := a_2
//     let a_3 := 3
//     a := a_3
//     let a_4 := 4
//     a := a_4
// }
