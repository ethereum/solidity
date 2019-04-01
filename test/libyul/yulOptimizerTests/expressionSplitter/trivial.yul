{
    mstore(add(calldataload(2), mload(3)), 8)
}
// ====
// step: expressionSplitter
// ----
// {
//     let _1 := 8
//     let _2 := 3
//     let _3 := mload(_2)
//     let _4 := 2
//     let _5 := calldataload(_4)
//     let _6 := add(_5, _3)
//     mstore(_6, _1)
// }
