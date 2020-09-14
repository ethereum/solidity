{
    // This is not fully simplified on purpose because we
    // need another split step in between. The full simplification
    // is tested in the fullSuite.
    let c := create(0, 0, 0x20)
    let a := and(c, 0xffffffffffffffffffffffffffffffffffffffff)
    let b := and(0xffffffffffffffffffffffffffffffffffffffff, create(0, 0, 0x20))
    sstore(a, b)
}
// ----
// step: expressionSimplifier
//
// {
//     let _1 := 0x20
//     let _2 := 0
//     let c := create(_2, _2, _1)
//     let _4 := 0xffffffffffffffffffffffffffffffffffffffff
//     let a := and(c, _4)
//     sstore(a, and(_4, create(_2, _2, _1)))
// }
