{
    let x := calldataload(0)
    let a := and(shl(x, 248), shl(x, 12))
    sstore(10, a)
}
// ====
// EVMVersion: >byzantium
// ----
// step: expressionSimplifier
//
// {
//     {
//         sstore(10, shl(calldataload(0), 8))
//     }
// }
