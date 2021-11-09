{
    let x := 5
    let y := x // y should not reuse the stack slot of x, since x is still used below
    sstore(y, x)
}
// ====
// stackOptimization: true
// ----
//     /* "":15:16   */
//   0x05
//     /* "":21:31   */
//   dup1
//     /* "":107:119   */
//   sstore
//     /* "":0:121   */
//   stop
