{
    let c
    let d
    c := calldataload(0)
    // This assignment will be overwritten in all branches and thus can be removed.
    d := 1
    if c {
        d := 2
    }
    d := 3
    mstore(0, d)
}
// ====
// step: redundantAssignEliminator
// ----
// {
//     let c
//     let d
//     c := calldataload(0)
//     if c { }
//     d := 3
//     mstore(0, d)
// }
