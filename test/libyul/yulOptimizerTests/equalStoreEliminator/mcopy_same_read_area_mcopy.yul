{
    let _0 := 0
    let _32 := 32
    let _64 := 64

    calldatacopy(0, 0, 0x60)

    mcopy(_0, _64, _32)
    mcopy(_32, _64, _32) // Not redundant. Writing to a different area.
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
//     mcopy(_0, _64, _32)
//     mcopy(_32, _64, _32)
// }
