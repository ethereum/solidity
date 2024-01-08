{
    calldatacopy(0, 0, 0x40)

    let _0 := 0
    let _32 := 32
    let _42 := 42

    mcopy(_0, _32, _32) // Not redundant. MLOAD reads it.
    let mem0 := mload(_0)
    mstore(_0, _42)

    sstore(_0, mem0)
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
//         let mem0 := mload(_0)
//         mstore(_0, _42)
//         sstore(_0, mem0)
//         return(0, 0x40)
//     }
// }
