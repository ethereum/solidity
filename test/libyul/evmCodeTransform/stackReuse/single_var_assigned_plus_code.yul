{ let x := 1 mstore(3, 4) }
// ====
// stackOptimization: true
// ----
// PUSH1 0x1
// POP
// PUSH1 0x4
// PUSH1 0x3
// MSTORE
