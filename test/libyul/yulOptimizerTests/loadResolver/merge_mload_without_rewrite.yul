{
    let b := mload(2)
    sstore(0, b)
    if calldataload(1) {
        mstore(2, 7)
    }
    sstore(0, mload(2))
}
// ----
// step: loadResolver
//
// {
//     {
//         let _1 := 2
//         let b := mload(_1)
//         let _2 := 0
//         sstore(_2, b)
//         if calldataload(1) { mstore(_1, 7) }
//         sstore(_2, mload(_1))
//     }
// }
