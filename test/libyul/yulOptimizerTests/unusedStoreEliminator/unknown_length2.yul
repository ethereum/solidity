{
    let a
    switch calldataload(0)
    case 0 { a := calldataload(9) }
    case 1 { a := calldataload(10) }

    calldatacopy(0x20, 0, a)
    let x := mload(0)
    sstore(0, x)
}
// ----
// step: unusedStoreEliminator
//
// {
//     {
//         let a_9
//         let a := a_9
//         switch calldataload(0)
//         case 0 {
//             a := calldataload(9)
//             let a_10 := a
//         }
//         case 1 {
//             let a_12 := a
//             a := calldataload(10)
//             let a_11 := a
//         }
//         let a_13 := a
//         let _5 := 0
//         let _6 := 0x20
//         sstore(0, mload(0))
//     }
// }
