{
    mstore(calldataload(0), calldataload(10))
    let t := mload(calldataload(10))
    let q := mload(calldataload(0))
    sstore(t, q)
}
// ====
// EVMVersion: >=shanghai
// ----
// step: loadResolver
//
// {
//     {
//         let _2 := calldataload(10)
//         mstore(calldataload(0), _2)
//         let t := mload(_2)
//         sstore(t, mload(calldataload(0)))
//     }
// }
