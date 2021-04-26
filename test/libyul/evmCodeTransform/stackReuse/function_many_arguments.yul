{
    function f(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12, a13, a14, a15, a16, a17, a18, a19, a20) -> x {
        mstore(0x0100, a1)
        mstore(0x0120, a2)
        mstore(0x0140, a3)
        mstore(0x0160, a4)
        mstore(0x0180, a5)
        mstore(0x01A0, a6)
        mstore(0x01C0, a7)
        mstore(0x01E0, a8)
        mstore(0x0200, a9)
        mstore(0x0220, a10)
        mstore(0x0240, a11)
        mstore(0x0260, a12)
        mstore(0x0280, a13)
        mstore(0x02A0, a14)
        mstore(0x02C0, a15)
        mstore(0x02E0, a16)
        mstore(0x0300, a17)
        mstore(0x0320, a18)
        mstore(0x0340, a19)
        x := a20
    }
}
// ====
// stackOptimization: true
// ----
// PUSH1 0x80
// JUMP
// JUMPDEST
// DUP1
// PUSH2 0x100
// MSTORE
// POP
// DUP1
// PUSH2 0x120
// MSTORE
// POP
// DUP1
// PUSH2 0x140
// MSTORE
// POP
// DUP1
// PUSH2 0x160
// MSTORE
// POP
// DUP1
// PUSH2 0x180
// MSTORE
// POP
// DUP1
// PUSH2 0x1A0
// MSTORE
// POP
// DUP1
// PUSH2 0x1C0
// MSTORE
// POP
// DUP1
// PUSH2 0x1E0
// MSTORE
// POP
// DUP1
// PUSH2 0x200
// MSTORE
// POP
// DUP1
// PUSH2 0x220
// MSTORE
// POP
// DUP1
// PUSH2 0x240
// MSTORE
// POP
// DUP1
// PUSH2 0x260
// MSTORE
// POP
// DUP1
// PUSH2 0x280
// MSTORE
// POP
// DUP1
// PUSH2 0x2A0
// MSTORE
// POP
// DUP1
// PUSH2 0x2C0
// MSTORE
// POP
// DUP1
// PUSH2 0x2E0
// MSTORE
// POP
// DUP1
// PUSH2 0x300
// MSTORE
// POP
// DUP1
// PUSH2 0x320
// MSTORE
// POP
// DUP1
// PUSH2 0x340
// MSTORE
// POP
// PUSH1 0x0
// DUP2
// SWAP1
// POP
// JUMPDEST
// SWAP2
// SWAP1
// POP
// JUMP
// JUMPDEST
