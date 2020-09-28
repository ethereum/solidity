{
    // This is not fully simplified on purpose because we
    // need another split step in between. The full simplification
    // is tested in the fullSuite.
    let x := calldataload(0)
    let a := and(0xff, shr(248, shl(248, shr(248, and(x, 0xf)))))
    let b := shl(12, shr(4, and(x, 0xf0f0)))
    let c := shl(12, shr(4, and(0xf0f0, x)))
    let d := shl(12, shr(255, and(0xf0f0, x)))
    let e := shl(255, shr(4, and(0xf0f0, x)))
    let f := shl(12, shr(256, and(0xf0f0, x)))
    let g := shl(256, shr(4, and(0xf0f0, x)))
    sstore(10, a)
    sstore(11, b)
    sstore(12, c)
    sstore(13, d)
    sstore(14, e)
    sstore(15, f)
    sstore(16, g)
}
// ====
// EVMVersion: >byzantium
// ----
// step: expressionSimplifier
//
// {
//     let x := calldataload(0)
//     let _2 := 0xf
//     let _5 := and(shr(248, x), 0)
//     let _10 := 0xff
//     let a := and(_5, 255)
//     let _14 := and(shr(4, x), 3855)
//     let _15 := 12
//     let b := shl(_15, _14)
//     let _19 := and(shr(4, x), 3855)
//     let c := shl(_15, _19)
//     let d := shl(_15, and(shr(255, x), 0))
//     let e := shl(_10, _19)
//     let f := 0
//     let g := 0
//     sstore(10, a)
//     sstore(11, b)
//     sstore(_15, c)
//     sstore(13, d)
//     sstore(14, e)
//     sstore(_2, f)
//     sstore(16, g)
// }
