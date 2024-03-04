{
    calldatacopy(0, 0, 0x40)

    let _0 := 0
    let _1 := 1
    let _32 := 32
    let _42 := 42
    let _123 := 123

    mstore(_0, _42)    // Redundant. MSTORE overwrites it.
    mcopy(_1, _1, _32) // Redundant (no-op)
    mstore(_0, _123)

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
//         let _1 := 1
//         let _32 := 32
//         let _42 := 42
//         let _123 := 123
//         mstore(_0, _42)
//         mcopy(_1, _1, _32)
//         mstore(_0, _123)
//         return(0, 0x40)
//     }
// }
