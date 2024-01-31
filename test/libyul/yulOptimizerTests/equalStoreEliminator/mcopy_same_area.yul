{
    let _0 := 0
    let _32 := 32

    calldatacopy(0, 0, 0x20)

    mcopy(_0, _0, _32) // Redundant (no-op)
}
// ====
// EVMVersion: >=cancun
// ----
// step: equalStoreEliminator
//
// {
//     let _0 := 0
//     let _32 := 32
//     calldatacopy(0, 0, 0x20)
//     mcopy(_0, _0, _32)
// }
