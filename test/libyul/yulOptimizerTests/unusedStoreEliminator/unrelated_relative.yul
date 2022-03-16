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
//         let _3 := 8
//         let _5 := add(c, 0x20)
//         sstore(0, mload(c))
//         let _8 := 9
//         let _9 := 20
//         let _11 := add(c, 0x20)
//     }
// }
