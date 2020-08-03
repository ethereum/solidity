{
    let a := calldataload(0)
    let b := byte(33, a)
    let c := byte(20, a)
    // create cannot be removed.
    let d := byte(33, create(0, 0, 0x20))
    sstore(7, a)
    sstore(8, b)
    sstore(9, c)
    sstore(10, d)
}
// ----
// step: expressionSimplifier
//
// {
//     let _1 := 0
//     let a := calldataload(_1)
//     let b := 0
//     let c := byte(20, a)
//     pop(create(_1, _1, 0x20))
//     let d := 0
//     sstore(7, a)
//     sstore(8, b)
//     sstore(9, c)
//     sstore(10, d)
// }
