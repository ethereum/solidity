{
    let x
    // Cannot be removed, because we might skip the loop
    x := 1
    for { } calldataload(0) { }
    {
        if callvalue() {
            x := 2 // is preserved because of break stmt below.
            break
        }
        x := 3
    }
    mstore(x, 0x42)
}
// ----
// redundantAssignEliminator
// {
//     let x
//     x := 1
//     for {
//     }
//     calldataload(0)
//     {
//     }
//     {
//         if callvalue()
//         {
//             x := 2
//             break
//         }
//         x := 3
//     }
//     mstore(x, 0x42)
// }
