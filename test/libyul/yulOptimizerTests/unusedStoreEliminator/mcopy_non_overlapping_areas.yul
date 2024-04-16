{
    calldatacopy(0, 0, 0x40)

    let _0 := 0
    let _32 := 32

    mcopy(_0, _32, _32) // Not redundant. Not being overwritten.

    return(0, 0x40)
}
// ====
// EVMVersion: >=cancun
// ----
// step: unusedStoreEliminator
//
// {
//     {
//         calldatacopy(0, 0, 0x40)
//         let _0 := 0
//         let _32 := 32
//         mcopy(_0, _32, _32)
//         return(0, 0x40)
//     }
// }
