{
    let a := shl(299, calldataload(0))
    let b := shr(299, calldataload(1))
    let c := shl(255, calldataload(2))
    let d := shr(255, calldataload(3))
    sstore(a, b)
    sstore(c, d)
}
// ====
// EVMVersion: >=constantinople
// ----
// step: expressionSimplifier
//
// {
//     let a := 0
//     let b := 0
//     let _8 := calldataload(2)
//     let _9 := 255
//     let c := shl(_9, _8)
//     let d := shr(_9, calldataload(3))
//     sstore(a, b)
//     sstore(c, d)
// }
