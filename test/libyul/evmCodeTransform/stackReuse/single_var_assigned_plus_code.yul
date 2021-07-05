{ let x := 1 mstore(3, 4) }
// ====
// stackOptimization: true
// ----
//     /* "":11:12   */
//   pop(0x01)
//     /* "":23:24   */
//   0x04
//     /* "":20:21   */
//   0x03
//     /* "":13:25   */
//   mstore
//   stop
