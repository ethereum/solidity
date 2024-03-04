{
    calldatacopy(0, 0, 0x40)

    let _0 := 0
    let _32 := 32
    let _42 := 42

    mcopy(_0, _32, _32) // Redundant. MSTORE overwrites it.
    mcopy(_0, _0, _32)  // Redundant (no-op)
    mstore(_32, _42)

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
//         let _42 := 42
//         mcopy(_0, _32, _32)
//         mcopy(_0, _0, _32)
//         mstore(_32, _42)
//         return(0, 0x40)
//     }
// }
