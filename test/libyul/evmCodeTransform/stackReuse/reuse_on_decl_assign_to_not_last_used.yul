{
    let x := 5
    let y := x // y should not reuse the stack slot of x, since x is still used below
    sstore(y, x)
}
// ====
// stackOptimization: true
// ----
// PUSH1 0x5
// DUP1
// DUP2
// DUP2
// SSTORE
// POP
// POP
