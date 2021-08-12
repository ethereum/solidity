{ for { let z := 0 } 1 { } { let x := 3 } let t := 2 }
// ====
// stackOptimization: true
// ----
//     /* "":17:18   */
//   pop(0x00)
// tag_1:
//     /* "":38:39   */
//   pop(0x03)
//   jump(tag_1)
