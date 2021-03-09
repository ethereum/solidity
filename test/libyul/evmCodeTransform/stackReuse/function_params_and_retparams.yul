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
// PUSH1 0x11
// JUMP
// JUMPDEST
// PUSH1 0x0
// PUSH1 0x0
// JUMPDEST
// SWAP5
// POP
// SWAP5
// SWAP3
// POP
// POP
// POP
// JUMP
// JUMPDEST
