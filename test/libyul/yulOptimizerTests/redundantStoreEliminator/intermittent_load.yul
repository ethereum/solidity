{
    sstore(0, calldataload(0))
    let x := sload(0)
    sstore(1, calldataload(2))
    sstore(7, x)
    sstore(0, calldataload(7))
}
// ----
// step: redundantStoreEliminator
//
// {
//     let _1 := 0
//     let x := sload(_1)
//     sstore(1, calldataload(2))
//     let _8 := 7
//     sstore(_8, x)
//     sstore(_1, calldataload(_8))
// }
