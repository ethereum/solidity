{
    // This does not optimize the masks away. Due to the way the expression simplifier
    // is built, it would have to create another `create2` opcode for the simplification
    // which would be fatal.
    let a := and(create2(0, 0, 0x20, 0), 0xffffffffffffffffffffffffffffffffffffffff)
    let b := and(0xffffffffffffffffffffffffffffffffffffffff, create2(0, 0, 0x20, 0))
    sstore(a, b)
}
// ====
// EVMVersion: >=constantinople
// ----
// step: fullSuite
//
// {
//     {
//         let _1 := sub(shl(160, 1), 1)
//         let _2 := 0
//         let a := and(create2(_2, _2, 0x20, _2), _1)
//         sstore(a, and(_1, create2(_2, _2, 0x20, _2)))
//     }
// }
