{
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
// step: fullSuite
//
// {
//     {
//         let x := calldataload(returndatasize())
//         let b := and(shl(8, x), 15790080)
//         sstore(10, returndatasize())
//         sstore(11, b)
//         sstore(12, b)
//         sstore(13, returndatasize())
//         sstore(14, and(shl(251, x), shl(255, 1)))
//         sstore(0xf, returndatasize())
//         sstore(16, returndatasize())
//     }
// }
