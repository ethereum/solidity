{
    let x := calldataload(0)
    let z := calldataload(1)
    let t := and(and(x, z), x)
    let w := or(or(x, z), x)
    sstore(t, w)
}
// ----
// step: expressionSimplifier
//
// {
//     let x := calldataload(0)
//     let z := calldataload(1)
//     let t := and(x, z)
//     sstore(t, or(x, z))
// }
