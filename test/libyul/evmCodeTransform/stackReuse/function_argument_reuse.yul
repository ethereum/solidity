{
    function f(a, b, c) -> x { pop(address()) sstore(a, c) pop(callvalue()) x := b }
}
// ====
// stackOptimization: true
// ----
// PUSH1 0x17
// JUMP
// JUMPDEST
// PUSH1 0x0
// ADDRESS
// POP
// DUP4
// DUP3
// SSTORE
// CALLVALUE
// POP
// DUP3
// SWAP1
// POP
// JUMPDEST
// SWAP4
// SWAP3
// POP
// POP
// POP
// JUMP
// JUMPDEST
