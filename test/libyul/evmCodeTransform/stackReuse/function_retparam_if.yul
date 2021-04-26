{
    function f() -> x { pop(address()) if 1 { pop(callvalue()) } }
}
// ====
// stackOptimization: true
// ----
// PUSH1 0x14
// JUMP
// JUMPDEST
// ADDRESS
// POP
// PUSH1 0x0
// PUSH1 0x1
// ISZERO
// PUSH1 0x10
// JUMPI
// CALLVALUE
// POP
// JUMPDEST
// JUMPDEST
// SWAP1
// JUMP
// JUMPDEST
