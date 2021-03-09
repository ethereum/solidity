{
    function f() -> x { pop(address()) leave pop(callvalue()) }
}
// ====
// stackOptimization: true
// ----
// PUSH1 0x10
// JUMP
// JUMPDEST
// ADDRESS
// POP
// PUSH1 0x0
// PUSH1 0xD
// JUMP
// CALLVALUE
// POP
// JUMPDEST
// SWAP1
// JUMP
// JUMPDEST
