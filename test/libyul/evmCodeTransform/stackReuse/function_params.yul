{
    function f(a, b) { }
}
// ====
// stackOptimization: true
// ----
// PUSH1 0x8
// JUMP
// JUMPDEST
// POP
// POP
// JUMPDEST
// JUMP
// JUMPDEST
