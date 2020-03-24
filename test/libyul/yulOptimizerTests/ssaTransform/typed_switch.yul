{
  let b:bool := true
  let c:bool := false
  switch b
  case true { c := true}
  case false { }
  let d: bool := c
}
// ====
// dialect: evmTyped
// ----
// step: ssaTransform
//
// {
//     let b:bool := true
//     let c_1:bool := false
//     let c:bool := c_1
//     switch b
//     case true {
//         let c_2:bool := true
//         c := c_2
//     }
//     case false { }
//     let c_3:bool := c
//     let d:bool := c_3
// }
