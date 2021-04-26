{
    sstore(0, calldataload(0))
    sstore(1, calldataload(2))
    sstore(0, calldataload(7))
}
// ----
// step: redundantStoreEliminator
//
// {
//     let _1 := 0
//     sstore(1, calldataload(2))
//     sstore(_1, calldataload(7))
// }
