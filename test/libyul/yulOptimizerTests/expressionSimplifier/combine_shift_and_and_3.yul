{
    // This is not fully simplified on purpose because we
    // need another split step in between. The full simplification
    // is tested in the fullSuite.
    let x := calldataload(0)
    let a := shl(12, shr(4, x))
    let b := shl(4, shr(12, x))
    let c := shr(12, shl(4, x))
    let d := shr(4, shl(12, x))
    let e := shl(150, shr(2, shl(150, x)))
    sstore(15, x)
    sstore(16, a)
    sstore(17, b)
    sstore(18, c)
    sstore(19, d)
    sstore(20, e)
}
// ====
// EVMVersion: >byzantium
// ----
// step: expressionSimplifier
//
// {
//     {
//         let x := calldataload(0)
//         let a := and(shl(8, x), 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff000)
//         let b := and(shr(8, x), 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff0)
//         let c := and(shr(8, x), 0x0fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
//         let d := and(shl(8, x), 0x0fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff00)
//         let _14 := 150
//         let e := shl(_14, and(shl(148, x), 0x3ffffffffffffffffffffffffff0000000000000000000000000000000000000))
//         sstore(15, x)
//         sstore(16, a)
//         sstore(17, b)
//         sstore(18, c)
//         sstore(19, d)
//         sstore(20, e)
//     }
// }
