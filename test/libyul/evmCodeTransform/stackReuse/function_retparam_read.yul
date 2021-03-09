{
    function f() -> x { pop(address()) sstore(0, x) pop(callvalue()) }
}
// ====
// stackOptimization: true
// ----
// PUSH1 0x11
// JUMP
// JUMPDEST
// PUSH1 0x0
// ADDRESS
// POP
// DUP1
// PUSH1 0x0
// SSTORE
// CALLVALUE
// POP
// JUMPDEST
// SWAP1
// JUMP
// JUMPDEST
