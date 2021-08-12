{
    let x := 5
    let y := x // y should reuse the stack slot of x
    sstore(y, y)
}
// ====
// stackOptimization: true
// ----
//     /* "":15:16   */
//   0x05
//     /* "":74:86   */
//   dup1
//   sstore
//     /* "":0:88   */
//   stop
