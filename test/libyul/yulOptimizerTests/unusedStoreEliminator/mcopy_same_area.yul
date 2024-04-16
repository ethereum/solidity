{
    calldatacopy(0, 0, 0x20)

    let _0 := 0
    let _32 := 32

    mcopy(_0, _0, _32) // Redundant (no-op)

    return(0, 0x20)
}
// ====
// EVMVersion: >=cancun
// ----
// step: unusedStoreEliminator
//
// {
//     {
//         calldatacopy(0, 0, 0x20)
//         let _0 := 0
//         mcopy(_0, _0, 32)
//         return(0, 0x20)
//     }
// }
