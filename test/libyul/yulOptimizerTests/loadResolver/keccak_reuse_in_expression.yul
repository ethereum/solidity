{
    let x := calldataload(0)
    sstore(keccak256(0, x), keccak256(0, x))
}
// ====
// EVMVersion: >=shanghai
// ----
// step: loadResolver
//
// {
//     {
//         let x := calldataload(0)
//         sstore(keccak256(0, x), keccak256(0, x))
//     }
// }
