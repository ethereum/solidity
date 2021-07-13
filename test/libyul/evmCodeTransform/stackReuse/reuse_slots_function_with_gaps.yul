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
//   swap2
//     /* "":136:150   */
//   mstore
//   tag_1
//     /* "":207:210   */
//   tag_2
//   jump	// in
// tag_1:
//   swap4
//   swap1
//   swap3
//   swap2
//   pop
//     /* "":211:224   */
//   mstore
//     /* "":225:237   */
//   mstore
//   stop
//     /* "":155:184   */
// tag_2:
//   0x00
//   0x00
//   0x00
//   0x00
//   swap4
//   jump	// out
