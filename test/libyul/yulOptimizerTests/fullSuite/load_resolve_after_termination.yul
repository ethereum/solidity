{
    let _1 := calldataload(0)
    mstore(0, _1)
    if _1 { mstore(0, 9) return(0, 32) }
    sstore(0, mload(0))
}
// ----
// step: fullSuite
//
// {
//     {
//         let _1 := 0
//         let _2 := calldataload(_1)
//         mstore(_1, _2)
//         if _2
//         {
//             mstore(_1, 9)
//             return(_1, 32)
//         }
//         sstore(_1, mload(_1))
//     }
// }
