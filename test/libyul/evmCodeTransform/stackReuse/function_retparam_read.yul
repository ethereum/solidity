{
    function f() -> x { pop(address()) sstore(0, x) pop(callvalue()) }
}
// ====
// stackOptimization: true
// ----
// PUSH1 0x11
// JUMP
// JUMPDEST
// ADDRESS
// POP
// PUSH1 0x0
// DUP1
// PUSH1 0x0
// SSTORE
// CALLVALUE
// POP
// JUMPDEST
// SWAP1
// JUMP
// JUMPDEST
