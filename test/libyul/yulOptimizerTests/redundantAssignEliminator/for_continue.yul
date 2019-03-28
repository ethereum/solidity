{
    let x
    // Can be removed, because x is reassigned after the loop
    x := 1
    for { } calldataload(0) { }
    {
        x := 2 // Will not be removed as if-condition can be false.
        if callvalue() {
            // This can be removed because x is overwritten both after the
            // loop at at the start of the next iteration.
            x := 3
            continue
        }
        mstore(x, 2)
    }
    x := 3
}
// ----
// redundantAssignEliminator
// {
//     let x
//     for {
//     }
//     calldataload(0)
//     {
//     }
//     {
//         x := 2
//         if callvalue()
//         {
//             continue
//         }
//         mstore(x, 2)
//     }
// }
