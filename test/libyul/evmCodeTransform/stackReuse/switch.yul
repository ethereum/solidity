{ let z := 0 switch z case 0 { let x := 2 let y := 3 } default { z := 3 } let t := 9 }
// ====
// stackOptimization: true
// ----
//     /* "":11:12   */
//   0x00
//     /* "":27:28   */
//   0x00
//   eq
//   tag_1
//   jumpi
// tag_2:
//     /* "":70:71   */
//   pop(0x03)
// tag_3:
//     /* "":83:84   */
//   0x09
//   stop
// tag_1:
//     /* "":40:41   */
//   pop(0x02)
//     /* "":51:52   */
//   pop(0x03)
//   jump(tag_3)
