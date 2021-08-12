{ for { let z := 0 } 1 { } { z := 8 let x := 3 } let t := 2 }
// ====
// stackOptimization: true
// ----
//     /* "":17:18   */
//   pop(0x00)
// tag_1:
//     /* "":34:35   */
//   pop(0x08)
//     /* "":45:46   */
//   pop(0x03)
//   jump(tag_1)
