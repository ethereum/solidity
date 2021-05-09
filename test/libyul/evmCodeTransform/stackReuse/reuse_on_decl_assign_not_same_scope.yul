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
// PUSH1 0x5
// DUP1
// DUP1
// DUP2
// SSTORE
// POP
// POP
