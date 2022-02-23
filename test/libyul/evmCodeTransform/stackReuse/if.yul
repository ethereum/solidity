// z is only removed after the if (after the jumpdest)
{ let z := mload(0) if z { let x := z } let t := 3 }
// ====
// stackOptimization: true
// ----
//     /* "":72:73   */
//   0x00
//     /* "":66:74   */
//   mload
//     /* "":75:94   */
//   dup1
//   tag_1
//   jumpi
//     /* "":55:107   */
// tag_2:
//     /* "":95:105   */
//   pop
//     /* "":104:105   */
//   0x03
//     /* "":55:107   */
//   stop
//     /* "":80:94   */
// tag_1:
//   jump(tag_2)
