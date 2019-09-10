{
    let x
    // Not referenced anywhere.
    x := 1
    switch calldataload(0)
    case 0 { mstore(0, 1) }
}
// ====
// step: redundantAssignEliminator
// ----
// {
//     let x
//     switch calldataload(0)
//     case 0 { mstore(0, 1) }
// }
