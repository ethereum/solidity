{
    // This is not fully simplified on purpose because we
    // need another split step in between. The full simplification
    // is tested in the fullSuite.
    let x := calldataload(0)
    let a := and(0xff, shr(248, shl(248, shr(248, x))))
    let b := shr(12, shl(8, and(x, 0xf0f0)))
    sstore(a, b)
}
// ====
// EVMVersion: >byzantium
// ----
// step: expressionSimplifier
//
// {
//     {
//         let x := calldataload(0)
//         let a := and(0xff, and(shr(248, x), 255))
//         sstore(a, shr(12, and(shl(8, x), 15790080)))
//     }
// }
