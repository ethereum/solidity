{
    let a := shl(299, calldataload(0))
    let b := shr(299, calldataload(1))
    let c := shl(255, calldataload(2))
    let d := shr(255, calldataload(3))
}
// ====
// EVMVersion: >byzantium
// step: expressionSimplifier
// ----
// {
//     let a := 0
//     let b := 0
//     let c := shl(255, calldataload(2))
//     let d := shr(255, calldataload(3))
// }
