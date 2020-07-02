{
  for { let x := 0 } 1 { x := 2 } {
    for { let y := 0 } 1 { y := 6 } {
    }
  }
}
// ----
// step: ssaTransform
//
// {
//     let x_1 := 0
//     let x := x_1
//     for { }
//     1
//     {
//         let x_7 := x
//         let x_2 := 2
//         x := x_2
//     }
//     {
//         let x_5 := x
//         let y_3 := 0
//         let y := y_3
//         for { }
//         1
//         {
//             let y_6 := y
//             let y_4 := 6
//             y := y_4
//         }
//         { }
//     }
// }
