{
    calldatacopy(0, 0, 0x80)

    let _0 := 0
    let _32 := 32
    let _64 := 64
    let _96 := 96

    // Sanity check: independent MCOPY is not affected
    mcopy(_0, _32, _32)  // Not redundant. Not being overwritten.
    mcopy(_64, _96, _32)

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
//         let _32 := 32
//         let _64 := 64
//         let _96 := 96
//         mcopy(_0, _32, _32)
//         mcopy(_64, _96, _32)
//         return(0, 0x80)
//     }
// }
