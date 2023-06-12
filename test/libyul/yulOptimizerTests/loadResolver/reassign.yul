{
    let a := calldataload(0)
    sstore(a, 6)
    a := calldataload(2)
    mstore(0, sload(a))
}
// ====
// EVMVersion: >=shanghai
// ----
// step: loadResolver
//
// {
//     {
//         let a := calldataload(0)
//         sstore(a, 6)
//         a := calldataload(2)
//         mstore(0, sload(a))
//     }
// }
