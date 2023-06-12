{
    let a := calldataload(0)
    let b := calldataload(0x20)
    let x := sub(a, b)
    if iszero(x) {
        sstore(0, 1)
    }
}
// ====
// EVMVersion: >=shanghai
// ----
// step: expressionSimplifier
//
// {
//     {
//         let a := calldataload(0)
//         if eq(a, calldataload(0x20)) { sstore(0, 1) }
//     }
// }
