{
    let x
    // Will be used in some and overwritten in others
    x := 1
    switch calldataload(0)
    case 0 { x := 2 }
    default { mstore(x, 1) }
    mstore(x, 0)
}
// ====
// step: redundantAssignEliminator
// ----
// {
//     let x
//     x := 1
//     switch calldataload(0)
//     case 0 {
//         x := 2
//     }
//     default {
//         mstore(x, 1)
//     }
//     mstore(x, 0)
// }
