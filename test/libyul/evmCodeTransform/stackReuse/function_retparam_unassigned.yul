{
    function f() -> x { pop(callvalue()) }
}
// ====
// stackOptimization: true
// ----
// PUSH1 0xB
// JUMP
// JUMPDEST
// PUSH1 0x0
// CALLVALUE
// POP
// JUMPDEST
// SWAP1
// JUMP
// JUMPDEST
