{
    let a := calldataload(0)
    let b := calldataload(0x20)
    let x := sub(a, b)
    if iszero(x) {
        sstore(0, 1)
    }
}
// ----
// step: expressionSimplifier
//
// {
//     {
//         let _1 := 0
//         let a := calldataload(_1)
//         if eq(a, calldataload(0x20)) { sstore(_1, 1) }
//     }
// }
