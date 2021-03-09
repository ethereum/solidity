{ for { let z := 0 } 1 { } { let x := 3 } let t := 2 }
// ====
// stackOptimization: true
// ----
// PUSH1 0x0
// POP
// JUMPDEST
// PUSH1 0x1
// ISZERO
// PUSH1 0x11
// JUMPI
// PUSH1 0x3
// POP
// JUMPDEST
// PUSH1 0x3
// JUMP
// JUMPDEST
// PUSH1 0x2
// POP
