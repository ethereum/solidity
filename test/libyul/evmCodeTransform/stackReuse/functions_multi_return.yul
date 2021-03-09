{
    function f(a, b) -> t { }
    function g() -> r, s { }
    let x := f(1, 2)
    x := f(3, 4)
    let y, z := g()
    y, z := g()
    let unused := 7
}
// ====
// stackOptimization: true
// ----
// PUSH1 0x15
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
// PUSH1 0x0
// PUSH1 0x0
// JUMPDEST
// SWAP1
// SWAP2
// JUMP
// JUMPDEST
// PUSH1 0x1F
// PUSH1 0x2
// PUSH1 0x1
// PUSH1 0x3
// JUMP
// JUMPDEST
// PUSH1 0x29
// PUSH1 0x4
// PUSH1 0x3
// PUSH1 0x3
// JUMP
// JUMPDEST
// SWAP1
// POP
// POP
// PUSH1 0x32
// PUSH1 0xC
// JUMP
// JUMPDEST
// PUSH1 0x38
// PUSH1 0xC
// JUMP
// JUMPDEST
// SWAP2
// POP
// SWAP2
// POP
// POP
// POP
// PUSH1 0x7
// POP
