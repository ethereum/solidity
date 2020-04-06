{
  let b:bool := true
  let c:bool := false
  for {} b {} {
      c := true
  }
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
//     for { } b { }
//     {
//         let c_3:bool := c
//         let c_2:bool := true
//         c := c_2
//     }
//     let c_4:bool := c
//     let d:bool := c_4
// }
