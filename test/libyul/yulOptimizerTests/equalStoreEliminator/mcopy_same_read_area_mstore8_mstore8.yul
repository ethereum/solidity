{
    let _0 := 0
    let _1 := 1
    let _31 := 31

    calldatacopy(0, 0, 0x40)
    let mem31 := mload(_31)

    mstore8(_0, mem31)  // Redundant. Overwritten by MSTORE.
    mcopy(_0, _0, _1)   // Redundant (no-op)
    mstore8(_0, mem31)
}
// ====
// EVMVersion: >=cancun
// ----
// step: equalStoreEliminator
//
// {
//     let _0 := 0
//     let _1 := 1
//     let _31 := 31
//     calldatacopy(0, 0, 0x40)
//     let mem31 := mload(_31)
//     mstore8(_0, mem31)
//     mcopy(_0, _0, _1)
//     mstore8(_0, mem31)
// }
