{
    let _0 := 0
    let _31 := 31

    calldatacopy(0, 0, 0x40)
    let mem31 := mload(_31)

    mstore8(_0, mem31)
    mstore8(_0, mem31) // Redundant with previous MSTORE8.
}
// ----
// step: equalStoreEliminator
//
// {
//     let _0 := 0
//     let _31 := 31
//     calldatacopy(0, 0, 0x40)
//     let mem31 := mload(_31)
//     mstore8(_0, mem31)
//     mstore8(_0, mem31)
// }
