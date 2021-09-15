{
  function f(x, y) {
    mstore(0x80, x)
    if calldataload(0) { sstore(y, y) }
  }
}
// ====
// stackOptimization: true
// ----
//     /* "":0:88   */
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
//     /* "":47:82   */
//   tag_2
//   jumpi
//     /* "":21:86   */
// tag_3:
//     /* "":4:86   */
//   pop
//   jump	// out
//     /* "":66:82   */
// tag_2:
//     /* "":68:80   */
//   dup1
//   sstore
//     /* "":66:82   */
//   codesize
//   jump(tag_3)
