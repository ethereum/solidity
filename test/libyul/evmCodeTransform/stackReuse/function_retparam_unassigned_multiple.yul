{
    function f() -> x, y, z { pop(callvalue()) }
}
// ====
// stackOptimization: true
// ----
// PUSH1 0x11
// JUMP
// JUMPDEST
// CALLVALUE
// POP
// PUSH1 0x0
// PUSH1 0x0
// PUSH1 0x0
// JUMPDEST
// SWAP1
// SWAP2
// SWAP3
// JUMP
// JUMPDEST
