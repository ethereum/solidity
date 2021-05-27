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
// step: redundantStoreEliminator
//
// {
//     let start := calldataload(0x10)
//     if calldataload(0)
//     {
//         let _4 := 7
//         mstore(add(start, 2), _4)
//         calldatacopy(start, 0, 0x20)
//     }
//     if calldataload(1)
//     {
//         let _11 := 9
//         let _13 := add(start, 2)
//         let _14 := 0x21
//         let _15 := 0
//         calldatacopy(add(start, 1), _15, _14)
//     }
//     if calldataload(2)
//     {
//         let _20 := 7
//         let _22 := add(start, 2)
//         calldatacopy(start, 0, 3)
//     }
//     if calldataload(3)
//     {
//         let _27 := 7
//         mstore8(add(start, 3), _27)
//         calldatacopy(start, 0, 3)
//     }
//     sstore(0, keccak256(start, 0x40))
// }
