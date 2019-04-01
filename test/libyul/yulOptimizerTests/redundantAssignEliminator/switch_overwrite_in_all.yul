{
    let x
    // Will be overwritten in all branches
    x := 1
    switch calldataload(0)
    case 0 { x := 2 }
    default { x := 3 }
    mstore(x, 0)
}
// ====
// step: redundantAssignEliminator
// ----
// {
//     let x
//     switch calldataload(0)
//     case 0 {
//         x := 2
//     }
//     default {
//         x := 3
//     }
//     mstore(x, 0)
// }
