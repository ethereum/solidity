{
    calldatacopy(0, 0, 0x60)

    let _0 := 0
    let _32 := 32
    let _42 := 42
    let _64 := 64
    let _123 := 123

    mstore(_32, _42) // Redundant. MCOPY overwrites it.
    mcopy(_0, _32, _64)
    mstore(_32, _123)

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
//         let _123 := 123
//         mstore(_32, _42)
//         mcopy(_0, _32, _64)
//         mstore(_32, _123)
//         return(0, 0x60)
//     }
// }
