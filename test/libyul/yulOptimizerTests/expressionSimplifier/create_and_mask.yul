{
    let a := and(create(0, 0, 0x20), 0xffffffffffffffffffffffffffffffffffffffff)
    let b := and(0xffffffffffffffffffffffffffffffffffffffff, create(0, 0, 0x20))
}
// ====
// step: expressionSimplifier
// ----
// {
//     let a := create(0, 0, 0x20)
//     let b := create(0, 0, 0x20)
// }
