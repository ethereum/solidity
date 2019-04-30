{
    let a := and(0xff, shr(248, calldataload(0)))
    let b := and(shr(248, calldataload(0)), 0xff)
    let c := and(shr(249, calldataload(0)), 0xfa)
    let d := and(shr(247, calldataload(0)), 0xff)
}
// ====
// EVMVersion: >=constantinople
// step: expressionSimplifier
// ----
// {
//     let a := shr(248, calldataload(0))
//     let b := shr(248, calldataload(0))
//     let c := and(shr(249, calldataload(0)), 0xfa)
//     let d := and(shr(247, calldataload(0)), 0xff)
// }
