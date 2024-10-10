{
  let b := true
  let c := false
  switch b
  case true { c := true}
  case false { }
  let d := c
}
// ----
// step: ssaTransform
//
// {
//     let b := true
//     let c_1 := false
//     let c := c_1
//     switch b
//     case true {
//         let c_2 := true
//         c := c_2
//     }
//     case false { }
//     let c_3 := c
//     let d := c_3
// }
