{ let a, b, c, d let x := 2 let y := 3 mstore(x, a) mstore(y, c) }
// ====
// stackOptimization: true
// ----
//     /* "":2:16   */
//   0x00
//   dup1
//   dup1
//   dup1
//     /* "":17:27   */
//   pop
//   swap2
//   swap1
//   pop
//     /* "":26:27   */
//   0x02
//     /* "":28:38   */
//   swap1
//     /* "":37:38   */
//   0x03
//     /* "":39:51   */
//   swap2
//   mstore
//     /* "":52:64   */
//   mstore
//     /* "":0:66   */
//   stop
