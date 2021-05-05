{
    if calldataload(0) {
        // not covered
        mstore(2, 7)
        calldatacopy(0, 0, 0x20)
    }
    if calldataload(1) {
        // covered
        mstore(2, 9)
        calldatacopy(1, 0, 0x21)
    }
    if calldataload(2) {
        // covered
        mstore8(2, 7)
        calldatacopy(0, 0, 3)
    }
    if calldataload(3) {
        // not covered
        mstore8(3, 7)
        calldatacopy(0, 0, 3)
    }
    sstore(0, keccak256(0, 0x40))
}
// ----
// step: unusedStoreEliminator
//
// {
//     {
//         if calldataload(0)
//         {
//             mstore(2, 7)
//             calldatacopy(0, 0, 0x20)
//         }
//         if calldataload(1)
//         {
//             let _10 := 9
//             let _11 := 2
//             calldatacopy(1, 0, 0x21)
//         }
//         if calldataload(2)
//         {
//             let _17 := 7
//             let _18 := 2
//             calldatacopy(0, 0, 3)
//         }
//         if calldataload(3)
//         {
//             mstore8(3, 7)
//             calldatacopy(0, 0, 3)
//         }
//         sstore(0, keccak256(0, 0x40))
//     }
// }
