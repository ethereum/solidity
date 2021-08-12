{ let z := 0 { pop(z) } let x := 1 }
// ====
// stackOptimization: true
// ----
//     /* "":11:12   */
//   0x00
//     /* "":15:21   */
//   pop
//     /* "":33:34   */
//   0x01
//   stop
