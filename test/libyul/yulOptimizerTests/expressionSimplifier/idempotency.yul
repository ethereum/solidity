{
    let x := calldataload(0)
    let z := calldataload(1)
    let t := and(and(x, z), x)
    let w := or(or(x, z), x)
}
// ----
// step: expressionSimplifier
//
// {
//     let x := calldataload(0)
//     let z := calldataload(1)
//     let t := and(x, z)
//     let w := or(x, z)
// }
