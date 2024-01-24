{
    calldatacopy(0, 0, 0x60)

    let _0 := 0
    let _31 := 31
    let _32 := 32
    let _42 := 42
    let _64 := 64
    let _123 := 123

    mstore(_0, _42) // Not redundant. MCOPY reads part of it.
    mcopy(_64, _31, _32)
    mstore(_0, _123)

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
//         let _31 := 31
//         let _32 := 32
//         let _42 := 42
//         let _64 := 64
//         let _123 := 123
//         mstore(_0, _42)
//         mcopy(_64, _31, _32)
//         mstore(_0, _123)
//         return(0, 0x60)
//     }
// }
