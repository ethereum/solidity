{
    let a := sub(calldataload(1), calldataload(0))
    sstore(0, a)
}
// ----
// step: expressionSimplifier
//
// {
//     let _1 := 0
//     sstore(_1, sub(calldataload(1), calldataload(_1)))
// }
