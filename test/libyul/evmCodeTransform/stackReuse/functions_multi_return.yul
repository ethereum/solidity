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
// PUSH1 0x14
// JUMP
// JUMPDEST
// POP
// POP
// PUSH1 0x0
// JUMPDEST
// SWAP1
// JUMP
// JUMPDEST
// PUSH1 0x0
// PUSH1 0x0
// JUMPDEST
// SWAP1
// SWAP2
// JUMP
// JUMPDEST
// PUSH1 0x1E
// PUSH1 0x2
// PUSH1 0x1
// PUSH1 0x3
// JUMP
// JUMPDEST
// PUSH1 0x28
// PUSH1 0x4
// PUSH1 0x3
// PUSH1 0x3
// JUMP
// JUMPDEST
// SWAP1
// POP
// POP
// PUSH1 0x31
// PUSH1 0xB
// JUMP
// JUMPDEST
// PUSH1 0x37
// PUSH1 0xB
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
