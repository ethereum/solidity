{
    let x := 5
    let y := add(x, 2) // y should reuse the stack slot of x
    sstore(y, y)
}
// ====
// stackOptimization: true
// ----
// PUSH1 0x5
// PUSH1 0x2
// DUP2
// ADD
// SWAP1
// POP
// DUP1
// DUP2
// SSTORE
// POP
