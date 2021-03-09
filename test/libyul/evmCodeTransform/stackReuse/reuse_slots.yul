{ let a, b, c, d let x := 2 let y := 3 mstore(x, a) mstore(y, c) }
// ====
// stackOptimization: true
// ----
// PUSH1 0x0
// PUSH1 0x0
// PUSH1 0x0
// PUSH1 0x0
// POP
// PUSH1 0x2
// SWAP2
// POP
// PUSH1 0x3
// DUP4
// DUP4
// MSTORE
// DUP2
// DUP2
// MSTORE
// POP
// POP
// POP
// POP
