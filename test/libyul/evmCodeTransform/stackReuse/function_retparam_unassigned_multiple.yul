{
    function f() -> x, y, z { pop(callvalue()) }
}
// ====
// stackOptimization: true
// ----
// PUSH1 0xF
// JUMP
// JUMPDEST
// CALLVALUE
// POP
// PUSH1 0x0
// PUSH1 0x0
// PUSH1 0x0
// JUMPDEST
// SWAP3
// JUMP
// JUMPDEST
