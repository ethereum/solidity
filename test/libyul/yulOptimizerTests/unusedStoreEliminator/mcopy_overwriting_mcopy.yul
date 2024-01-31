{
    calldatacopy(0, 0, 0x80)

    let _0 := 0
    let _64 := 64

    mcopy(_0, _64, _64) // Redundant. MCOPY overwrites it.
    mcopy(_0, _64, _64)

    return(0, 0x80)
}
// ====
// EVMVersion: >=cancun
// ----
// step: unusedStoreEliminator
//
// {
//     {
//         calldatacopy(0, 0, 0x80)
//         let _0 := 0
//         let _64 := 64
//         mcopy(_0, _64, _64)
//         mcopy(_0, _64, _64)
//         return(0, 0x80)
//     }
// }
