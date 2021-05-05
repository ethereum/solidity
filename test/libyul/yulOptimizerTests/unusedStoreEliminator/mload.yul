{
    mstore(0, 5)
    let x := mload(0)
    mstore(0, 8)
    let y := mload(0)
    sstore(0, y)
}
// ----
// step: unusedStoreEliminator
//
// {
//     {
//         mstore(0, 5)
//         let x := mload(0)
//         mstore(0, 8)
//         sstore(0, mload(0))
//     }
// }
