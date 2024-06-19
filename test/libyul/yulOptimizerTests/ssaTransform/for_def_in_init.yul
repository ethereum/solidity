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
//         let x_4 := x
//         let x_2 := 2
//         x := x_2
//     }
//     {
//         let x_3 := x
//         let y_1 := 0
//         let y := y_1
//         for { }
//         1
//         {
//             let y_3 := y
//             let y_2 := 6
//             y := y_2
//         }
//         { }
//     }
// }
