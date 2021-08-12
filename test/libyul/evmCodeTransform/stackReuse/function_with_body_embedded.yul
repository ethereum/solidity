{
    let b := 3
    function f(a, r) -> t {
        // r could be removed right away, but a cannot - this is not implemented, though
        let x := a a := 3 t := a
    }
    b := 7
}
// ====
// stackOptimization: true
// ----
//     /* "":15:16   */
//   pop(0x03)
//     /* "":182:183   */
//   0x07
//   stop
//     /* "":21:172   */
// tag_1:
//   swap1
//   pop
//   pop
//     /* "":158:159   */
//   0x03
//   swap1
//     /* "":21:172   */
//   jump	// out
