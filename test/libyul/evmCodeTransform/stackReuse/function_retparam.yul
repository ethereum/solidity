{
    function f() -> x, y { }
}
// ====
// stackOptimization: true
// ----
// PUSH1 0xC
// JUMP
// JUMPDEST
// PUSH1 0x0
// PUSH1 0x0
// JUMPDEST
// SWAP1
// SWAP2
// JUMP
// JUMPDEST
