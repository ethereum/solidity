{
    let x := 5
    let y := x // y should reuse the stack slot of x
    sstore(y, y)
}
// ====
// stackOptimization: true
// ----
// PUSH1 0x5
// DUP1
// SWAP1
// POP
// DUP1
// DUP2
// SSTORE
// POP
