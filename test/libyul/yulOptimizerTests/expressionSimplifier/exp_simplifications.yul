{
    let t := calldataload(0)
    sstore(0, exp(0, t))
    sstore(1, exp(1, t))
    sstore(2, exp(2, t))
    // The following should not be simplified
    sstore(3, exp(8, t))
}
// ====
// EVMVersion: >=constantinople
// ----
// step: expressionSimplifier
//
// {
//     let _1 := 0
//     let t := calldataload(_1)
//     sstore(_1, iszero(t))
//     sstore(1, 1)
//     let _8 := 2
//     sstore(_8, shl(t, 1))
//     sstore(3, exp(8, t))
// }
