{
    let x := 5
    {
        let y := x // y should not reuse the stack slot of x, since x is not in the same scope
        sstore(y, y)
    }
}
// ====
// stackOptimization: true
// ----
//     /* "":15:16   */
//   0x05
//   dup1
//     /* "":126:138   */
//   sstore
//   stop
