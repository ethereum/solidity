{ let a, b, c, d let x := 2 let y := 3 mstore(x, a) mstore(y, c) }
// ====
// stackOptimization: true
// ----
//     /* "":2:16   */
//   0x00
//   dup1
//   dup1
//   dup1
//   pop
//   swap2
//   swap1
//   pop
//     /* "":26:27   */
//   0x02
//   swap1
//     /* "":37:38   */
//   0x03
//   swap2
//     /* "":39:51   */
//   mstore
//     /* "":52:64   */
//   mstore
//   stop
