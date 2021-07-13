{
    function f(a, b) -> t { }
    function g() -> r, s { }
    let x := f(1, 2)
    x := f(3, 4)
    let y, z := g()
    y, z := g()
    let unused := 7
}
// ====
// stackOptimization: true
// ----
//   tag_1
//     /* "":79:80   */
//   0x02
//     /* "":76:77   */
//   0x01
//     /* "":74:81   */
//   tag_2
//   jump	// in
// tag_1:
//   pop
//   tag_3
//     /* "":96:97   */
//   0x04
//     /* "":93:94   */
//   0x03
//     /* "":91:98   */
//   tag_2
//   jump	// in
// tag_3:
//   pop
//   tag_4
//     /* "":115:118   */
//   tag_5
//   jump	// in
// tag_4:
//   pop
//   pop
//   tag_6
//     /* "":131:134   */
//   tag_5
//   jump	// in
// tag_6:
//   pop
//   pop
//     /* "":153:154   */
//   0x07
//   stop
//     /* "":6:31   */
// tag_2:
//   pop
//   pop
//   0x00
//   swap1
//   jump	// out
//     /* "":36:60   */
// tag_5:
//   0x00
//   0x00
//   swap2
//   jump	// out
