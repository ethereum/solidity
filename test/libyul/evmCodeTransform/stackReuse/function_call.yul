{
    let b := f(1, 2)
    function f(a, r) -> t { }
    b := f(3, 4)
}
// ====
// stackOptimization: true
// ----
//   tag_1
//     /* "":20:21   */
//   0x02
//     /* "":17:18   */
//   0x01
//     /* "":15:22   */
//   tag_2
//   jump	// in
// tag_1:
//   pop
//   tag_3
//     /* "":67:68   */
//   0x04
//     /* "":64:65   */
//   0x03
//     /* "":62:69   */
//   tag_2
//   jump	// in
// tag_3:
//   stop
//     /* "":27:52   */
// tag_2:
//   pop
//   pop
//   0x00
//   swap1
//   jump	// out
