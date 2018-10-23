{
    let x
    // Will NOT be overwritten in all branches
    x := 1
    switch calldataload(0)
    case 0 { x := 2 }
    mstore(x, 0)
}
// ----
// redundantAssignEliminator
// {
//     let x
//     x := 1
//     switch calldataload(0)
//     case 0 {
//         x := 2
//     }
//     mstore(x, 0)
// }
