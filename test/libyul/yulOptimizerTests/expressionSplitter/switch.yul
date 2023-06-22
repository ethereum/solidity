{
    let x := 8
    switch add(2, calldataload(0))
    case 0 { sstore(0, mload(2)) }
    default { mstore(0, mload(3)) }
    x := add(mload(3), 4)
}
// ----
// step: expressionSplitter
//
// {
//     let x := 8
//     let _1 := calldataload(0)
//     let _2 := 2
//     let _3 := add(_2, _1)
//     switch _3
//     case 0 {
//         let _4 := 2
//         let _5 := mload(_4)
//         sstore(0, _5)
//     }
//     default {
//         let _6 := 3
//         let _7 := mload(_6)
//         mstore(0, _7)
//     }
//     let _8 := 4
//     let _9 := 3
//     let _10 := mload(_9)
//     x := add(_10, _8)
// }
