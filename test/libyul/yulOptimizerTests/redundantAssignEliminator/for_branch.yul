{
    let x
    let y
    // Cannot be removed, because we might skip the loop
    x := 1
    for { } calldataload(0) { }
    {
        // Cannot be removed
        x := 2
        // Can be removed
        y := 3
    }
    y := 8
    mstore(x, 0)
}
// ====
// step: redundantAssignEliminator
// ----
// {
//     let x
//     let y
//     x := 1
//     for {
//     }
//     calldataload(0)
//     {
//     }
//     {
//         x := 2
//     }
//     mstore(x, 0)
// }
