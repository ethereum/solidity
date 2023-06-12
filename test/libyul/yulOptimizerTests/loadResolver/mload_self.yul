{
    let x := calldataload(0)
    x := mload(x)
    let y := mload(x)
    sstore(0, y)
}
// ====
// EVMVersion: >=shanghai
// ----
// step: loadResolver
//
// {
//     {
//         let x := calldataload(0)
//         x := mload(x)
//         sstore(0, mload(x))
//     }
// }
