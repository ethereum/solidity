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
//         let a_5
//         let a := a_5
//         switch calldataload(0)
//         case 0 {
//             a := calldataload(9)
//             let a_6 := a
//         }
//         case 1 {
//             let a_8 := a
//             a := calldataload(10)
//             let a_7 := a
//         }
//         calldatacopy(0x20, 0, a)
//         sstore(0, mload(0))
//     }
// }
