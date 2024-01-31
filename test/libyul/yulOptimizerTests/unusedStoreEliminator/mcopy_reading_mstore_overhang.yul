{
    calldatacopy(0, 0, 0x120)

    let _0 := 0
    let _32 := 32
    let _42 := 42
    let _96 := 96
    let _123 := 123

    mstore(_32, _42) // Not redundant. MCOPY reads it.
    mcopy(_96, _0, _96)
    mstore(_32, _123)

    return(0, 0x120)
}
// ====
// EVMVersion: >=cancun
// ----
// step: unusedStoreEliminator
//
// {
//     {
//         calldatacopy(0, 0, 0x120)
//         let _0 := 0
//         let _32 := 32
//         let _42 := 42
//         let _96 := 96
//         let _123 := 123
//         mstore(_32, _42)
//         mcopy(_96, _0, _96)
//         mstore(_32, _123)
//         return(0, 0x120)
//     }
// }
