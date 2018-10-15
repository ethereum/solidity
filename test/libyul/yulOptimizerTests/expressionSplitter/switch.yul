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
//     let _1 := 0
//     let _2 := calldataload(_1)
//     let _3 := 2
//     let _4 := add(_3, _2)
//     switch _4
//     case 0 {
//         let _5 := 2
//         let _6 := mload(_5)
//         let _7 := 0
//         sstore(_7, _6)
//     }
//     default {
//         let _8 := 3
//         let _9 := mload(_8)
//         let _10 := 0
//         mstore(_10, _9)
//     }
//     let _11 := 4
//     let _12 := 3
//     let _13 := mload(_12)
//     x := add(_13, _11)
// }
