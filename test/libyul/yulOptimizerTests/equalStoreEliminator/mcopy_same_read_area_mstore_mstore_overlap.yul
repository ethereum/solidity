{
    let _0 := 0
    let _1 := 1
    let _32 := 32

    calldatacopy(0, 0, 0x20)
    let mem32 := mload(_32)

    mstore(_0, mem32)
    mcopy(_1, _1, _32) // Redundant (no-op)
    mstore(_0, mem32)  // Redundant with previous MSTORE.
}
// ====
// EVMVersion: >=cancun
// ----
// step: equalStoreEliminator
//
// {
//     let _0 := 0
//     let _1 := 1
//     let _32 := 32
//     calldatacopy(0, 0, 0x20)
//     let mem32 := mload(_32)
//     mstore(_0, mem32)
//     mcopy(_1, _1, _32)
//     mstore(_0, mem32)
// }
