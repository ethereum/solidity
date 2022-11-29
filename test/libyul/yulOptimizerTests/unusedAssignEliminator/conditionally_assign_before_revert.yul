{
    let a := calldataload(0)
    if calldataload(1) {
        // this can be removed
        a := 2
        revert(0, 0)
    }
    sstore(0, a)
}
// ----
// step: unusedAssignEliminator
//
// {
//     let a := calldataload(0)
//     if calldataload(1) { revert(0, 0) }
//     sstore(0, a)
// }
