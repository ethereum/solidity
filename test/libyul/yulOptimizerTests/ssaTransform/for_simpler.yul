{
  let b := true
  let c := false
  for {} b {} {
      c := true
  }
  let d := c
}
// ----
// step: ssaTransform
//
// {
//     let b := true
//     let c_1 := false
//     let c := c_1
//     for { } b { }
//     {
//         let c_3 := c
//         let c_2 := true
//         c := c_2
//     }
//     let c_4 := c
//     let d := c_4
// }
