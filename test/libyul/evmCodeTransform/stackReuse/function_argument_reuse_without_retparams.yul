{
  function f(x, y) {
    mstore(0x80, x)
    if calldataload(0) { sstore(y, y) }
  }
}
// ====
// stackOptimization: true
// ----
// PUSH1 0x17
// JUMP
// JUMPDEST
// DUP1
// PUSH1 0x80
// MSTORE
// POP
// PUSH1 0x0
// CALLDATALOAD
// ISZERO
// PUSH1 0x13
// JUMPI
// DUP1
// DUP2
// SSTORE
// JUMPDEST
// POP
// JUMPDEST
// JUMP
// JUMPDEST
