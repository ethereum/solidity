{
    let x := 1
    let y := 1
    for { } calldataload(0) { }
    {
        y := 9
        if callvalue() {
            x := 2
            y := 2 // will be removed
            break
            x := 7 // after break, we start with fresh state.
        }
        if eq(callvalue(), 3) {
            x := 12
            y := 12 // will be removed
            continue
            x := 17 // after continue, we start with fresh state.
            y := 9
        }
        x := 3
        mstore(y, 3)
    }
    mstore(x, 0x42)
}
// ====
// step: redundantAssignEliminator
// ----
// {
//     let x := 1
//     let y := 1
//     for {
//     }
//     calldataload(0)
//     {
//     }
//     {
//         y := 9
//         if callvalue()
//         {
//             x := 2
//             break
//         }
//         if eq(callvalue(), 3)
//         {
//             x := 12
//             continue
//             y := 9
//         }
//         x := 3
//         mstore(y, 3)
//     }
//     mstore(x, 0x42)
// }
