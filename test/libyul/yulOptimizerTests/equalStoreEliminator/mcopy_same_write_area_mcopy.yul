{
    let _0 := 0
    let _32 := 32
    let _64 := 64

    calldatacopy(0, 0, 0x60)

    mcopy(_0, _32, _32)
    mcopy(_0, _64, _32) // Not redundant. Copying from a different spot.
}
// ====
// EVMVersion: >=cancun
// ----
// step: equalStoreEliminator
//
// {
//     let _0 := 0
//     let _32 := 32
//     let _64 := 64
//     calldatacopy(0, 0, 0x60)
//     mcopy(_0, _32, _32)
//     mcopy(_0, _64, _32)
// }
