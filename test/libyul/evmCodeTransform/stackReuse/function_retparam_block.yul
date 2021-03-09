{
    function f() -> x { pop(address()) { pop(callvalue()) } }
}
// ====
// stackOptimization: true
// ----
// PUSH1 0xD
// JUMP
// JUMPDEST
// PUSH1 0x0
// ADDRESS
// POP
// CALLVALUE
// POP
// JUMPDEST
// SWAP1
// JUMP
// JUMPDEST
