{
    let b := mload(2)
    mstore(0, b)
    if calldataload(1) {
        mstore(2, 7)
        mstore(0, 7)
        return(0, 0)
    }
    sstore(0, mload(0))
}
// ----
// step: loadResolver
//
// {
//     {
//         let _1 := 2
//         let b := mload(_1)
//         let _2 := 0
//         mstore(_2, b)
//         if calldataload(1)
//         {
//             let _5 := 7
//             mstore(_1, _5)
//             mstore(_2, _5)
//             return(_2, _2)
//         }
//         sstore(_2, b)
//     }
// }
