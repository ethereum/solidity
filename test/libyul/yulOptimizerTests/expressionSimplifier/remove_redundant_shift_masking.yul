{
    let a := and(0xff, shr(248, calldataload(0)))
    let b := and(shr(248, calldataload(0)), 0xff)
    let c := and(shr(249, calldataload(0)), 0xfa)
    let d := and(shr(247, calldataload(0)), 0xff)
    sstore(a, b)
    sstore(c, d)
}
// ====
// EVMVersion: >=constantinople
// ----
// step: expressionSimplifier
//
// {
//     {
//         let _1 := calldataload(0)
//         let _2 := 0xff
//         let a := shr(248, _1)
//         let b := shr(248, _1)
//         let c := and(shr(249, _1), 0xfa)
//         let d := and(shr(247, _1), _2)
//         sstore(a, b)
//         sstore(c, d)
//     }
// }
