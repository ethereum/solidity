{
    let zero := 0
    mstore(zero, 5)
    let x := mload(zero)
    mstore(zero, 8)
    let y := mload(zero)
    sstore(zero, y)
}
// ----
// step: unusedStoreEliminator
//
// {
//     {
//         let zero := 0
//         mstore(zero, 5)
//         let x := mload(zero)
//         mstore(zero, 8)
//         sstore(zero, mload(zero))
//     }
// }
