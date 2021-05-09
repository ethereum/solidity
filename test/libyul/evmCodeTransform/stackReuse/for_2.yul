{ for { let z := 0 } 1 { } { z := 8 let x := 3 } let t := 2 }
// ====
// stackOptimization: true
// ----
// PUSH1 0x0
// JUMPDEST
// PUSH1 0x1
// ISZERO
// PUSH1 0x14
// JUMPI
// PUSH1 0x8
// SWAP1
// POP
// PUSH1 0x3
// POP
// JUMPDEST
// PUSH1 0x2
// JUMP
// JUMPDEST
// POP
// PUSH1 0x2
// POP
