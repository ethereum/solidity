{
    mstore(0, 30)
    if calldataload(0) {
        mstore(0, 20)
    }
    let val := keccak256(0, 32)
    sstore(0, val)
}
// ====
// EVMVersion: >=shanghai
// ----
// step: loadResolver
//
// {
//     {
//         mstore(0, 30)
//         if calldataload(0) { mstore(0, 20) }
//         sstore(0, keccak256(0, 32))
//     }
// }
