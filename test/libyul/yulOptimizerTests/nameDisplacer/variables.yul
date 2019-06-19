{ { let illegal1 := 1 } { let illegal2 := 2 let illegal3, illegal4 } }
// ====
// step: nameDisplacer
// ----
// {
//     { let illegal1_1 := 1 }
//     {
//         let illegal2_2 := 2
//         let illegal3_3, illegal4_4
//     }
// }
