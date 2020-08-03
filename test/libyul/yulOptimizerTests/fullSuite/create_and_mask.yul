{
    // This does not optimize the masks away. Due to the way the expression simplifier
    // is built, it would have to create another `create` opcode for the simplification
    // which would be fatal.
    let a := and(create(0, 0, 0x20), 0xffffffffffffffffffffffffffffffffffffffff)
    let b := and(0xffffffffffffffffffffffffffffffffffffffff, create(0, 0, 0x20))
    sstore(a, b)
}
// ====
// EVMVersion: >=istanbul
// ----
// step: fullSuite
//
// {
//     {
//         let _1 := sub(shl(160, 1), 1)
//         let a := and(create(0, 0, 0x20), _1)
//         sstore(a, and(_1, create(0, 0, 0x20)))
//     }
// }
