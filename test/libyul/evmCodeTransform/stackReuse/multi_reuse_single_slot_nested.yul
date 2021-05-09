{ let x := 1 x := 6 { let y := 2 y := 4 } }
// ====
// stackOptimization: true
// ----
// PUSH1 0x1
// PUSH1 0x6
// SWAP1
// POP
// POP
// PUSH1 0x2
// PUSH1 0x4
// SWAP1
// POP
// POP
