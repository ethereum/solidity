{
    function f(a, b, c) -> x { pop(address()) sstore(a, c) pop(callvalue()) x := b }
}
// ====
// stackOptimization: true
// ----
// PUSH1 0x17
// JUMP
// JUMPDEST
// ADDRESS
// POP
// DUP3
// DUP2
// SSTORE
// POP
// CALLVALUE
// POP
// PUSH1 0x0
// DUP2
// SWAP1
// POP
// JUMPDEST
// SWAP3
// SWAP2
// POP
// POP
// JUMP
// JUMPDEST
