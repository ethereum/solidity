{
    let x := 1
    let y := 1
    let a := 7
    let b := 9
    for { } calldataload(0) { }
    {
        y := 9
        mstore(a, 7)
        if callvalue() {
            x := 2
            for {} calldataload(1) {}
            {
                a := 2 // can be removed
                if eq(x, 3) {
                    b := 3 // cannot be removed
                    y := 2 // will be removed
                    continue
                }
            }
            mstore(b, 2)
            break
        }
        if eq(callvalue(), 3) {
            x := 12
            y := 12
            continue
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
//     let a := 7
//     let b := 9
//     for {
//     }
//     calldataload(0)
//     {
//     }
//     {
//         y := 9
//         mstore(a, 7)
//         if callvalue()
//         {
//             x := 2
//             for {
//             }
//             calldataload(1)
//             {
//             }
//             {
//                 if eq(x, 3)
//                 {
//                     b := 3
//                     continue
//                 }
//             }
//             mstore(b, 2)
//             break
//         }
//         if eq(callvalue(), 3)
//         {
//             x := 12
//             continue
//         }
//         x := 3
//         mstore(y, 3)
//     }
//     mstore(x, 0x42)
// }
