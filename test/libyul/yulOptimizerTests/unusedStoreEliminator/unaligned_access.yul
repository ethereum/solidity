{
    let zero := 0
    mstore(zero, 0x1234)
    mstore(4, 0x456)
    revert(zero, 5)
}
// ----
// step: unusedStoreEliminator
//
// {
//     {
//         let zero := 0
//         mstore(zero, 0x1234)
//         mstore(4, 0x456)
//         revert(zero, 5)
//     }
// }
