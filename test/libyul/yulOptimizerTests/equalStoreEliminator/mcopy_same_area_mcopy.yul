{
    let _0 := 0
    let _32 := 32

    calldatacopy(0, 0, 0x40)

    mcopy(_0, _32, _32)
    mcopy(_0, _32, _32) // Redundant with previous MCOPY.
}
// ====
// EVMVersion: >=cancun
// ----
// step: equalStoreEliminator
//
// {
//     let _0 := 0
//     let _32 := 32
//     calldatacopy(0, 0, 0x40)
//     mcopy(_0, _32, _32)
//     mcopy(_0, _32, _32)
// }
