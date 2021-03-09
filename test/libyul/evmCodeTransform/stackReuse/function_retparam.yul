{
    function f() -> x, y { }
}
// ====
// stackOptimization: true
// ----
// PUSH1 0xB
// JUMP
// JUMPDEST
// PUSH1 0x0
// PUSH1 0x0
// JUMPDEST
// SWAP2
// JUMP
// JUMPDEST
