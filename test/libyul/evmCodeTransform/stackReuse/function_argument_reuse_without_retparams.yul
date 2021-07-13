{
  function f(x, y) {
    mstore(0x80, x)
    if calldataload(0) { sstore(y, y) }
  }
}
// ====
// stackOptimization: true
// ----
//   stop
//     /* "":4:86   */
// tag_1:
//     /* "":34:38   */
//   0x80
//     /* "":27:42   */
//   mstore
//     /* "":63:64   */
//   0x00
//     /* "":50:65   */
//   calldataload
//   tag_2
//   jumpi
// tag_3:
//   pop
//     /* "":4:86   */
//   jump	// out
// tag_2:
//   dup1
//     /* "":68:80   */
//   sstore
//   pc
//   jump(tag_3)
