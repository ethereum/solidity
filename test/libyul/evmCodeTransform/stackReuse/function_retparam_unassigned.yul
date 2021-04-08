{
    function f() -> x { pop(callvalue()) }
}
// ====
// stackOptimization: true
// ----
// PUSH1 0xB
// JUMP
// JUMPDEST
// CALLVALUE
// POP
// PUSH1 0x0
// JUMPDEST
// SWAP1
// JUMP
// JUMPDEST
