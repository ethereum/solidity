{
    // This is not fully simplified on purpose because we
    // need another split step in between. The full simplification
    // is tested in the fullSuite.
    let c := create(0, 0, 0x20)
    let a := and(c, 0xffffffffffffffffffffffffffffffffffffffff)
    let b := and(0xffffffffffffffffffffffffffffffffffffffff, create(0, 0, 0x20))
    sstore(a, b)
}
// ====
// EVMVersion: >=shanghai
// ----
// step: expressionSimplifier
//
// {
//     {
//         let _1 := 0x20
//         let c := create(0, 0, _1)
//         let _2 := 0xffffffffffffffffffffffffffffffffffffffff
//         let a := and(c, _2)
//         sstore(a, and(_2, create(0, 0, _1)))
//     }
// }
