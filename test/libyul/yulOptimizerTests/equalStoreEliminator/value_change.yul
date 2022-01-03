{
    let x := calldataload(0)
    let y := calldataload(32)
    sstore(x, y)
    y := calldataload(64)
    // cannot be removed
    sstore(x, y)
}
// ----
// step: equalStoreEliminator
//
// {
//     let x := calldataload(0)
//     let y := calldataload(32)
//     sstore(x, y)
//     y := calldataload(64)
//     sstore(x, y)
// }
