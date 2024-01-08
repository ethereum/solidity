{
    let _0 := 0
    let _1 := 1
    let _31 := 31
    let _32 := 32
    let _33 := 33

    calldatacopy(0, 0, 0x40)
    let mem32 := mload(_32)

    mstore(_0, mem32)
    mcopy(_1, _33, _31) // Not redundant. MCOPY does not copy all of it.
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
//     let _32 := 32
//     let _33 := 33
//     calldatacopy(0, 0, 0x40)
//     let mem32 := mload(_32)
//     mstore(_0, mem32)
//     mcopy(_1, _33, _31)
// }
