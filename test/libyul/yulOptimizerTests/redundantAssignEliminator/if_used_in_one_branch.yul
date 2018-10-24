{
    let c
    let d
    c := calldataload(0)
    d := 1
    if c {
        // Uses the assignment above
        d := d
    }
    d := 3
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
//     }
//     d := 3
//     mstore(0, d)
// }
