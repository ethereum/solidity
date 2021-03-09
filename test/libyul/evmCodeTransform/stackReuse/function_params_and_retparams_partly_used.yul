{
    function f(a, b, c, d) -> x, y { b := 3 let s := 9 y := 2 mstore(s, y) }
}
// ====
// stackOptimization: true
// ----
// PUSH1 0x1F
// JUMP
// JUMPDEST
// PUSH1 0x0
// PUSH1 0x0
// PUSH1 0x3
// SWAP4
// POP
// PUSH1 0x9
// PUSH1 0x2
// SWAP2
// POP
// DUP2
// DUP2
// MSTORE
// POP
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
