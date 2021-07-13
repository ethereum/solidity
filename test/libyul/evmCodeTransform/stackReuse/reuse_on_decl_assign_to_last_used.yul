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
//   dup1
//     /* "":74:86   */
//   sstore
//   stop
