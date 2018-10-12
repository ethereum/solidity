{
    let x := 8
    switch add(2, calldataload(0))
    case 0 { sstore(0, mload(2)) }
    default { mstore(0, mload(3)) }
    x := add(mload(3), 4)
}
// ----
// expressionSplitter
// {
//     let x := 8
//     let _1 := calldataload(0)
//     let _2 := add(2, _1)
//     switch _2
//     case 0 {
//         let _3 := mload(2)
//         sstore(0, _3)
//     }
//     default {
//         let _4 := mload(3)
//         mstore(0, _4)
//     }
//     let _5 := mload(3)
//     x := add(_5, 4)
// }
