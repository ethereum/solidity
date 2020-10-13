{
    let x := 7

    let y1, y2, y3, y4, y5, y6, y7, y8, y9, y10, y11, y12, y13, y14, y15 := verbatim_0i_15o("\x60\x42") // the verbatim will show up as PUSH1 42

    // last use of x - the slot of x will be marked as unused, but not popped, since it is not at the stack top
    sstore(0,x)

    // If the slot of x is blindly reused, this will fail.
    let z1, z2 := verbatim_0i_2o("\x60\x43") // will show up as PUSH1 43

    // prevent the z's from being popped immediately after their declaration above.
    mstore(1, z1)
    mstore(1, z2)

    // use all y's to prevent them from being popped immediately after their declaration above
    sstore(1, y1)
    sstore(1, y2)
    sstore(1, y3)
    sstore(1, y4)
    sstore(1, y5)
    sstore(1, y6)
    sstore(1, y7)
    sstore(1, y8)
    sstore(1, y9)
    sstore(1, y10)
    sstore(1, y11)
    sstore(1, y12)
    sstore(1, y13)
    sstore(1, y14)
    sstore(1, y15)
}
// ====
// stackOptimization: true
// ----
// PUSH1 0x7
// PUSH1 0x42
// DUP16
// PUSH1 0x0
// SSTORE
// PUSH1 0x43
// DUP2
// PUSH1 0x1
// MSTORE
// DUP1
// PUSH1 0x1
// MSTORE
// POP
// POP
// DUP15
// PUSH1 0x1
// SSTORE
// DUP14
// PUSH1 0x1
// SSTORE
// DUP13
// PUSH1 0x1
// SSTORE
// DUP12
// PUSH1 0x1
// SSTORE
// DUP11
// PUSH1 0x1
// SSTORE
// DUP10
// PUSH1 0x1
// SSTORE
// DUP9
// PUSH1 0x1
// SSTORE
// DUP8
// PUSH1 0x1
// SSTORE
// DUP7
// PUSH1 0x1
// SSTORE
// DUP6
// PUSH1 0x1
// SSTORE
// DUP5
// PUSH1 0x1
// SSTORE
// DUP4
// PUSH1 0x1
// SSTORE
// DUP3
// PUSH1 0x1
// SSTORE
// DUP2
// PUSH1 0x1
// SSTORE
// DUP1
// PUSH1 0x1
// SSTORE
// POP
// POP
// POP
// POP
// POP
// POP
// POP
// POP
// POP
// POP
// POP
// POP
// POP
// POP
// POP
// POP
