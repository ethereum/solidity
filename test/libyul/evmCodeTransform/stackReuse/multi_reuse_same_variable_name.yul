{ let z := mload(0) { let x := 1 x := 6 z := x } { let x := 2 z := x x := 4 } }
// ====
// stackOptimization: true
// ----
// PUSH1 0x0
// MLOAD
// PUSH1 0x1
// PUSH1 0x6
// SWAP1
// POP
// DUP1
// SWAP2
// POP
// POP
// PUSH1 0x2
// DUP1
// SWAP2
// POP
// PUSH1 0x4
// SWAP1
// POP
// POP
// POP
