// z is only removed after the if (after the jumpdest)
{ let z := mload(0) if z { let x := z } let t := 3 }
// ====
// stackOptimization: true
// ----
// PUSH1 0x0
// MLOAD
// DUP1
// ISZERO
// PUSH1 0xA
// JUMPI
// DUP1
// POP
// JUMPDEST
// POP
// PUSH1 0x3
// POP
