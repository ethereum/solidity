{
    let start := calldataload(0x10)
    if calldataload(0) {
        // not covered
        mstore(add(start, 2), 7)
        calldatacopy(start, 0, 0x20)
    }
    if calldataload(1) {
        // covered
        mstore(add(start, 2), 9)
        calldatacopy(add(start, 1), 0, 0x21)
    }
    if calldataload(2) {
        // covered
        mstore8(add(start, 2), 7)
        calldatacopy(start, 0, 3)
    }
    if calldataload(3) {
        // not covered
        mstore8(add(start, 3), 7)
        calldatacopy(start, 0, 3)
    }
    sstore(0, keccak256(start, 0x40))
}
// ----
// step: unusedStoreEliminator
//
// {
//     {
//         let start := calldataload(0x10)
//         if calldataload(0)
//         {
//             let _3 := 7
//             mstore(add(start, 2), _3)
//             calldatacopy(start, 0, 0x20)
//         }
//         if calldataload(1)
//         {
//             let _9 := 9
//             mstore(add(start, 2), _9)
//             let _12 := 0x21
//             calldatacopy(add(start, 1), 0, _12)
//         }
//         if calldataload(2)
//         {
//             let _17 := 7
//             mstore8(add(start, 2), _17)
//             calldatacopy(start, 0, 3)
//         }
//         if calldataload(3)
//         {
//             let _23 := 7
//             mstore8(add(start, 3), _23)
//             calldatacopy(start, 0, 3)
//         }
//         sstore(0, keccak256(start, 0x40))
//     }
// }
