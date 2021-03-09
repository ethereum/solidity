// This does not re-use the parameters for the return parameters
// We do not expect parameters to be fully unused, so the stack
// layout for a function is still fixed, even though parameters
// can be re-used.
{
    function f(a, b, c, d) -> x, y { }
}
// ====
// stackOptimization: true
// ----
// PUSH1 0xF
// JUMP
// JUMPDEST
// POP
// POP
// POP
// POP
// PUSH1 0x0
// PUSH1 0x0
// JUMPDEST
// SWAP2
// JUMP
// JUMPDEST
