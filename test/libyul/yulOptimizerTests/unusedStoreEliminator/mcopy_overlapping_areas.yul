{
    calldatacopy(0, 0, 0x60)

    let _0 := 0
    let _32 := 32
    let _64 := 64

    mcopy(_0, _32, _64) // Not redundant. Not being overwritten.

    return(0, 0x60)
}
// ====
// EVMVersion: >=cancun
// ----
// step: unusedStoreEliminator
//
// {
//     {
//         calldatacopy(0, 0, 0x60)
//         let _0 := 0
//         let _32 := 32
//         mcopy(_0, _32, 64)
//         return(0, 0x60)
//     }
// }
