{
    let x
    // Can be removed, because x is not used after the loop.
    x := 1
    for { } calldataload(0) { mstore(x, 0x42) }
    {
        if callvalue() {
            x := 2 // is preserved because of continue stmt below.
            continue
        }
        x := 3
    }
}
// ====
// step: redundantAssignEliminator
// ----
// {
//     let x
//     for { } calldataload(0) { mstore(x, 0x42) }
//     {
//         if callvalue()
//         {
//             x := 2
//             continue
//         }
//         x := 3
//     }
// }
