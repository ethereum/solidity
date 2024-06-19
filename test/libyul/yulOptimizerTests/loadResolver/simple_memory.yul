{
    mstore(calldataload(0), calldataload(10))
    let t := mload(calldataload(10))
    let q := mload(calldataload(0))
    sstore(t, q)
}
// ----
// step: loadResolver
//
// {
//     {
//         let _1 := calldataload(10)
//         mstore(calldataload(0), _1)
//         sstore(mload(_1), _1)
//     }
// }
