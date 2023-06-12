{
    let x := calldataload(0)
    let a := keccak256(0, x)
    sstore(a, 2)
    let t := mload(2)
    let b := keccak256(0, x)
    sstore(b, 3)
}
// ====
// EVMVersion: >=shanghai
// ----
// step: loadResolver
//
// {
//     {
//         let x := calldataload(0)
//         sstore(keccak256(0, x), 2)
//         sstore(keccak256(0, x), 3)
//     }
// }
