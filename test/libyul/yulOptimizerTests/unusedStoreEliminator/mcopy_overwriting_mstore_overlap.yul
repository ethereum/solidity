{
    calldatacopy(0, 0, 0x60)

    let _0 := 0
    let _1 := 1
    let _32 := 32
    let _42 := 42
    let _64 := 64

    mstore(_0, _42) // Not redundant. MCOPY does not overwrite all of it.
    mcopy(_1, _64, _32)

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
//         let _1 := 1
//         let _32 := 32
//         let _42 := 42
//         let _64 := 64
//         mstore(_0, _42)
//         mcopy(_1, _64, _32)
//         return(0, 0x60)
//     }
// }
