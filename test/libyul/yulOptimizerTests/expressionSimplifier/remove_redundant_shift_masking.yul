{
    let a := and(0xff, shr(248, calldataload(0)))
    let b := and(shr(248, calldataload(0)), 0xff)
    let c := and(shr(249, calldataload(0)), 0xfa)
    let d := and(shr(247, calldataload(0)), 0xff)
    sstore(a, b)
    sstore(c, d)
}
// ====
// EVMVersion: >=shanghai
// ----
// step: expressionSimplifier
//
// {
//     {
//         let _2 := calldataload(0)
//         let _5 := 0xff
//         let a := shr(248, _2)
//         let b := shr(248, calldataload(0))
//         let c := and(shr(249, calldataload(0)), 0xfa)
//         let d := and(shr(247, calldataload(0)), _5)
//         sstore(a, b)
//         sstore(c, d)
//     }
// }
