{
    // No mload removal because of msize
    mstore(calldataload(0), msize())
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
//         mstore(calldataload(0), msize())
//         let t := mload(calldataload(10))
//         sstore(t, mload(calldataload(0)))
//     }
// }
