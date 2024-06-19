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
//             let _1 := 7
//             mstore(add(start, 2), _1)
//             calldatacopy(start, 0, 0x20)
//         }
//         if calldataload(1)
//         {
//             let _2 := 9
//             mstore(add(start, 2), _2)
//             let _3 := 0x21
//             let _4 := 0
//             calldatacopy(add(start, 1), _4, _3)
//         }
//         if calldataload(2)
//         {
//             let _5 := 7
//             mstore8(add(start, 2), _5)
//             calldatacopy(start, 0, 3)
//         }
//         if calldataload(3)
//         {
//             let _6 := 7
//             mstore8(add(start, 3), _6)
//             calldatacopy(start, 0, 3)
//         }
//         sstore(0, keccak256(start, 0x40))
//     }
// }
