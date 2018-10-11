{
    mstore(add(calldataload(2), mload(3)), 8)
}
// ----
// expressionSplitter
// {
//     let _1 := mload(3)
//     let _2 := calldataload(2)
//     let _3 := add(_2, _1)
//     mstore(_3, 8)
// }
