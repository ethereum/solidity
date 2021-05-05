{
    let _1 := 0
    if callvalue() { revert(_1, _1) }
    mstore(_1, shl(224, 0x4e487b71))
    mstore(4, 0x32)
    revert(_1, 0x24)
}
// ====
// EVMVersion: >=constantinople
// ----
// step: unusedStoreEliminator
//
// {
//     {
//         let _1 := 0
//         if callvalue() { revert(_1, _1) }
//         mstore(_1, shl(224, 0x4e487b71))
//         mstore(4, 0x32)
//         revert(_1, 0x24)
//     }
// }
