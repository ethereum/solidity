{
    // Only x3 is actually used, the slots of
    // x1 and x2 will be reused right away.
    let x1 := 5 let x2 := 6 let x3 := 7
    mstore(x1, x2)
    function f() -> x, y, z, t {}
    let a, b, c, d := f() mstore(x3, a) mstore(c, d)
}
// ====
// stackOptimization: true
// ----
//     /* "":106:107   */
//   0x05
//     /* "":118:119   */
//   0x06
//     /* "":130:131   */
//   0x07
//     /* "":136:150   */
//   swap2
//   mstore
//     /* "":207:210   */
//   tag_2
//   tag_1
//   jump	// in
// tag_2:
//     /* "":211:224   */
//   swap4
//   swap1
//   swap3
//   swap2
//   pop
//   mstore
//     /* "":225:237   */
//   mstore
//     /* "":0:239   */
//   stop
//     /* "":155:184   */
// tag_1:
//     /* "":174:175   */
//   0x00
//     /* "":177:178   */
//   0x00
//     /* "":180:181   */
//   0x00
//     /* "":171:172   */
//   0x00
//     /* "":155:184   */
//   swap4
//   jump	// out
