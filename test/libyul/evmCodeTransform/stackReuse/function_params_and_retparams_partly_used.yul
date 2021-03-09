{
    function f(a, b, c, d) -> x, y { b := 3 let s := 9 y := 2 mstore(s, y) }
}
// ====
// stackOptimization: true
// ----
// PUSH1 0x1D
// JUMP
// JUMPDEST
// POP
// PUSH1 0x3
// SWAP1
// POP
// POP
// POP
// POP
// PUSH1 0x0
// PUSH1 0x0
// PUSH1 0x9
// PUSH1 0x2
// SWAP3
// POP
// DUP3
// DUP2
// MSTORE
// POP
// JUMPDEST
// SWAP2
// JUMP
// JUMPDEST
