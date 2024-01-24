{
    calldatacopy(0, 0, 0x60)

    mcopy(0x20, 0x40, 0x20)

    return(0, 0x60)
}
// ====
// EVMVersion: >=cancun
// ----
// step: expressionSimplifier
//
// {
//     {
//         let _1 := 0x60
//         let _2 := 0
//         calldatacopy(_2, _2, _1)
//         let _4 := 0x20
//         mcopy(_4, 0x40, _4)
//         return(_2, _1)
//     }
// }
