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
//         let a_1
//         let a := a_1
//         switch calldataload(0)
//         case 0 {
//             a := calldataload(9)
//             let a_2 := a
//         }
//         case 1 {
//             let a_4 := a
//             a := calldataload(10)
//             let a_3 := a
//         }
//         let a_5 := a
//         let _1 := 0
//         let _2 := 0x20
//         sstore(0, mload(0))
//     }
// }
