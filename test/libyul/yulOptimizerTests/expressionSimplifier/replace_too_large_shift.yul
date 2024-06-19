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
//     {
//         let a := 0
//         let b := 0
//         let _1 := calldataload(2)
//         let _2 := 255
//         let c := shl(_2, _1)
//         let d := shr(_2, calldataload(3))
//         sstore(a, b)
//         sstore(c, d)
//     }
// }
