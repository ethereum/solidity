{
    let x := calldataload(0)
    let a := shl(12, shr(4, x))
    let b := shl(4, shr(12, x))
    let c := shr(12, shl(4, x))
    let d := shr(4, shl(12, x))
    let e := shl(150, shr(2, shl(150, x)))
}
// ====
// EVMVersion: >byzantium
// step: expressionSimplifier
// ----
// {
//     let x := calldataload(0)
//     let a := and(shl(8, x), 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff000)
//     let b := and(shr(8, x), 0xfffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff0)
//     let c := and(shr(8, x), 0x0fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff)
//     let d := and(shl(8, x), 0x0fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff00)
//     let e := 0
// }
