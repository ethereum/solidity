{
    // Only x3 is actually used, the slots of
    // x1 and x2 will be reused right away.
    let x1 := 5 let x2 := 6 let x3 := 7
    mstore(x1, x2)
    function f() -> x, y, z, t {}
    let a, b, c, d := f() mstore(x3, a) mstore(c, d)
}
// ====
// stackOptimization: true
// ----
// PUSH1 0x5
// PUSH1 0x6
// PUSH1 0x7
// DUP2
// DUP4
// MSTORE
// PUSH1 0x1B
// JUMP
// JUMPDEST
// PUSH1 0x0
// PUSH1 0x0
// PUSH1 0x0
// PUSH1 0x0
// JUMPDEST
// SWAP1
// SWAP2
// SWAP3
// SWAP4
// JUMP
// JUMPDEST
// PUSH1 0x21
// PUSH1 0xC
// JUMP
// JUMPDEST
// SWAP6
// POP
// SWAP4
// POP
// POP
// DUP1
// DUP3
// MSTORE
// POP
// POP
// DUP2
// DUP2
// MSTORE
// POP
// POP
