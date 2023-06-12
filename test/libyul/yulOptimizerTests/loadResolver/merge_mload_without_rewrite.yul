{
    let b := mload(2)
    sstore(0, b)
    if calldataload(1) {
        mstore(2, 7)
    }
    sstore(0, mload(2))
}
// ====
// EVMVersion: >=shanghai
// ----
// step: loadResolver
//
// {
//     {
//         let _1 := 2
//         sstore(0, mload(_1))
//         if calldataload(1) { mstore(_1, 7) }
//         sstore(0, mload(_1))
//     }
// }
