{
    let x
    // Cannot be removed, because we might run the loop only once
    x := 1
    for { } calldataload(0) { }
    {
        mstore(x, 2)
        // Cannot be removed because of the line above
        x := 2
    }
    x := 3
}
// ====
// step: redundantAssignEliminator
// ----
// {
//     let x
//     x := 1
//     for { } calldataload(0) { }
//     {
//         mstore(x, 2)
//         x := 2
//     }
// }
