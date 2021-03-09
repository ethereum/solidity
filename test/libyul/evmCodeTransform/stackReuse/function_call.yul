{
    let b := f(1, 2)
    function f(a, r) -> t { }
    b := f(3, 4)
}
// ====
// stackOptimization: true
// ----
// PUSH1 0x9
// PUSH1 0x2
// PUSH1 0x1
// PUSH1 0xD
// JUMP
// JUMPDEST
// PUSH1 0x16
// JUMP
// JUMPDEST
// PUSH1 0x0
// JUMPDEST
// SWAP3
// SWAP2
// POP
// POP
// JUMP
// JUMPDEST
// PUSH1 0x20
// PUSH1 0x4
// PUSH1 0x3
// PUSH1 0xD
// JUMP
// JUMPDEST
// SWAP1
// POP
// POP
