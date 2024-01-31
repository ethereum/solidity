{
    calldatacopy(0, 0, 0x60)

    let _0 := 0
    let _32 := 32
    let _64 := 64

    mcopy(_0, _32, _32) // Redundant. MCOPY overwrites it.
    mcopy(_0, _64, _32)

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
//         let _64 := 64
//         mcopy(_0, _32, _32)
//         mcopy(_0, _64, _32)
//         return(0, 0x60)
//     }
// }
