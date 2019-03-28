{
    let x := 1
    let y := 1
    let z := 1
    for { } calldataload(0) { mstore(x, 2) mstore(z, 2) }
    {
        y := 3
        switch callvalue()
        case 0 {
            x := 2
            y := 2
            z := 2
            break
        }
        case 1 {
            x := 3
            y := 3
            z := 3
            continue
        }
        case 2 {
            x := 4
            y := 4
            z := 4
            break
        }
        case 3 {
            x := 5
            y := 5
            z := 5
            continue
        }
        mstore(y, 9)
    }
    mstore(x, 0x42)
}
// ----
// redundantAssignEliminator
// {
//     let x := 1
//     let y := 1
//     let z := 1
//     for {
//     }
//     calldataload(0)
//     {
//         mstore(x, 2)
//         mstore(z, 2)
//     }
//     {
//         y := 3
//         switch callvalue()
//         case 0 {
//             x := 2
//             break
//         }
//         case 1 {
//             x := 3
//             z := 3
//             continue
//         }
//         case 2 {
//             x := 4
//             break
//         }
//         case 3 {
//             x := 5
//             z := 5
//             continue
//         }
//         mstore(y, 9)
//     }
//     mstore(x, 0x42)
// }
