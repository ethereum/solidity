{
    calldatacopy(0, 0, 0x20)

    let _0 := 0
    let _1 := 1
    let _42 := 42
    let _123 := 123

    mstore8(_0, _42)  // Redundant. MSTORE8 overwrites it.
    mcopy(_0, _0, _1) // Redundant (no-op)
    mstore8(_0, _123)

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
//         let _1 := 1
//         let _42 := 42
//         let _123 := 123
//         mstore8(_0, _42)
//         mcopy(_0, _0, _1)
//         mstore8(_0, _123)
//         return(0, 0x20)
//     }
// }
