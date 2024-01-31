{
    calldatacopy(0, 0, 0x20)

    let _0 := 0
    let _42 := 42
    let _123 := 123

    mstore(_0, _42) // Redundant. MSTORE overwrites it.
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
//         let _42 := 42
//         mstore(_0, 123)
//         return(0, 0x20)
//     }
// }
