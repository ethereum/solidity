{
    calldatacopy(0, 0, 0x60)

    let _0 := 0
    let _32 := 32
    let _42 := 42
    let _64 := 64

    // Sanity check: independent MCOPY and MSTORE are not affected
    mcopy(_0, _32, _32) // Not redundant. Not being overwritten.
    mstore(_64, _42)

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
//         let _42 := 42
//         let _64 := 64
//         mcopy(_0, _32, _32)
//         mstore(_64, _42)
//         return(0, 0x60)
//     }
// }
