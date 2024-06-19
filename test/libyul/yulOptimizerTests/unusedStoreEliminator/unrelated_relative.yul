{
    let c := calldataload(0)
    mstore(c, 4)
    mstore(add(c, 0x20), 8)
    sstore(0, mload(c))
    mstore(c, 9)
    mstore(add(c, 0x20), 20)
}
// ----
// step: unusedStoreEliminator
//
// {
//     {
//         let c := calldataload(0)
//         mstore(c, 4)
//         let _1 := 8
//         let _2 := add(c, 0x20)
//         sstore(0, mload(c))
//         let _3 := 9
//         let _4 := 20
//         let _5 := add(c, 0x20)
//     }
// }
