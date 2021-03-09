{
    function f() -> x, y, z, t {}
    let a, b, c, d := f() let x1 := 2 let y1 := 3 mstore(x1, a) mstore(y1, c)
}
// ====
// stackOptimization: true
// ----
// PUSH1 0x12
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
// PUSH1 0x18
// PUSH1 0x3
// JUMP
// JUMPDEST
// POP
// PUSH1 0x2
// SWAP2
// POP
// PUSH1 0x3
// DUP4
// DUP4
// MSTORE
// DUP2
// DUP2
// MSTORE
// POP
// POP
// POP
// POP
