{
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
// step: fullSuite
//
// {
//     {
//         let x := calldataload(0)
//         let _1 := shl(8, x)
//         let _2 := shr(8, x)
//         sstore(15, x)
//         sstore(16, and(_1, not(4095)))
//         sstore(17, and(_2, sub(shl(248, 1), 16)))
//         sstore(18, and(_2, sub(shl(244, 1), 1)))
//         sstore(19, and(_1, sub(shl(252, 1), 256)))
//         sstore(20, 0)
//     }
// }
