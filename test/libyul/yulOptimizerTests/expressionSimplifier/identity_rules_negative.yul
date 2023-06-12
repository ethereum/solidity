{
    let a := sub(calldataload(1), calldataload(0))
    sstore(0, a)
}
// ====
// EVMVersion: >=shanghai
// ----
// step: expressionSimplifier
//
// {
//     {
//         sstore(0, sub(calldataload(1), calldataload(0)))
//     }
// }
