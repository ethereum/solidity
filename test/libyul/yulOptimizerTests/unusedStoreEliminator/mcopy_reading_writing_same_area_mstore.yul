{
    calldatacopy(0, 0, 0x20)

    let _0 := 0
    let _32 := 32
    let _42 := 42
    let _123 := 123

    mstore(_0, _42)    // Redundant. MSTORE overwrites it.
    mcopy(_0, _0, _32) // Redundant (no-op)
    mstore(_0, _123)

    return(0, 0x20)
}
// ====
// EVMVersion: >=cancun
// ----
// step: unusedStoreEliminator
//
// {
//     {
//         let _1 := 0x20
//         let _2 := 0
//         let _3 := 0
//         let _0 := 0
//         let _32 := 32
//         let _42 := 42
//         let _123 := 123
//         mstore(_0, _42)
//         mcopy(_0, _0, _32)
//         mstore(_0, _123)
//         return(0, 0x20)
//     }
// }
