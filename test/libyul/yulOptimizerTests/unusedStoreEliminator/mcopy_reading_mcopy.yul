{
    calldatacopy(0, 0, 0x60)

    let _0 := 0
    let _32 := 32
    let _42 := 42
    let _64 := 64

    mcopy(_32, _64, _32) // Not redundant. MCOPY reads it.
    mcopy(_0, _32, _32)
    mstore(_32, _42)

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
//         mcopy(_32, 64, _32)
//         mcopy(_0, _32, _32)
//         mstore(_32, _42)
//         return(0, 0x60)
//     }
// }
