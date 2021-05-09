{ let z := 0 switch z case 0 { let x := 2 let y := 3 } default { z := 3 } let t := 9 }
// ====
// stackOptimization: true
// ----
// PUSH1 0x0
// DUP1
// PUSH1 0x0
// DUP2
// EQ
// PUSH1 0x11
// JUMPI
// PUSH1 0x3
// SWAP2
// POP
// PUSH1 0x18
// JUMP
// JUMPDEST
// PUSH1 0x2
// POP
// PUSH1 0x3
// POP
// JUMPDEST
// POP
// POP
// PUSH1 0x9
// POP
