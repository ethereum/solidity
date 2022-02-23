{
    let x := calldataload(0)
    let a := and(0xff, shr(248, shl(248, shr(248, x))))
    let b := shr(12, shl(8, and(x, 0xf0f0)))
    sstore(a, b)
}
// ====
// EVMVersion: >byzantium
// ----
// step: fullSuite
//
// {
//     {
//         let x := calldataload(0)
//         sstore(shr(248, x), and(shr(4, x), 3855))
//     }
// }
