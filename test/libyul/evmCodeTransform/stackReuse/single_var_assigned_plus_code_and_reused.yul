{ let x := 1 mstore(3, 4) pop(mload(x)) }
// ====
// stackOptimization: true
// ----
// PUSH1 0x1
// PUSH1 0x4
// PUSH1 0x3
// MSTORE
// DUP1
// MLOAD
// POP
// POP
