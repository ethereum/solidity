{
    let t := calldataload(0)
    sstore(0, exp(0, t))
    sstore(1, exp(1, t))
    sstore(2, exp(2, t))
    // The following should not be simplified
    sstore(3, exp(8, t))
    sstore(4, exp(115792089237316195423570985008687907853269984665640564039457584007913129639935, t))
}
// ====
// EVMVersion: >=constantinople
// ----
// step: expressionSimplifier
//
// {
//     {
//         let _1 := 0
//         let t := calldataload(_1)
//         sstore(_1, iszero(t))
//         sstore(1, 1)
//         let _8 := 2
//         sstore(_8, shl(t, 1))
//         sstore(3, exp(8, t))
//         sstore(4, sub(iszero(and(t, 1)), and(t, 1)))
//     }
// }
