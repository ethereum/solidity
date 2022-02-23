{
    let b := mload(2)
    if calldataload(1) {
        mstore(2, 7)
        // Re-writing the old value, should allow to eliminate the load below.
        mstore(2, b)
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
//         if calldataload(1)
//         {
//             mstore(_1, 7)
//             mstore(_1, b)
//         }
//         sstore(0, b)
//     }
// }
