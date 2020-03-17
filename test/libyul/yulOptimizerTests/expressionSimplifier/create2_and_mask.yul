{
    let a := and(create2(0, 0, 0x20, 0), 0xffffffffffffffffffffffffffffffffffffffff)
    let b := and(0xffffffffffffffffffffffffffffffffffffffff, create2(0, 0, 0x20, 0))
}
// ====
// EVMVersion: >=constantinople
// ----
// step: expressionSimplifier
//
// {
//     let a := create2(0, 0, 0x20, 0)
//     let b := create2(0, 0, 0x20, 0)
// }
