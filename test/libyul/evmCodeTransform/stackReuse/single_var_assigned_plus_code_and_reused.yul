{ let x := 1 mstore(3, 4) pop(mload(x)) }
// ====
// stackOptimization: true
// ----
//     /* "":11:12   */
//   0x01
//     /* "":23:24   */
//   0x04
//     /* "":20:21   */
//   0x03
//     /* "":13:25   */
//   mstore
//     /* "":30:38   */
//   mload
//     /* "":26:39   */
//   pop
//   stop
