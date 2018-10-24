{
    let c
    let d
    c := calldataload(0)
    d := 1
    if c {
        d := 2
    }
    // This enforces that none of the assignments above can be removed.
    mstore(0, d)
}
// ----
// redundantAssignEliminator
// {
//     let c
//     let d
//     c := calldataload(0)
//     d := 1
//     if c
//     {
//         d := 2
//     }
//     mstore(0, d)
// }
