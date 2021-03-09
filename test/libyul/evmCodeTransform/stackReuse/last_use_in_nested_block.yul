{ let z := 0 { pop(z) } let x := 1 }
// ====
// stackOptimization: true
// ----
// PUSH1 0x0
// DUP1
// POP
// POP
// PUSH1 0x1
// POP
