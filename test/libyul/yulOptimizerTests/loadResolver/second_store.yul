{
    let x := calldataload(1)
    sstore(x, 7)
    sstore(calldataload(0), 6)
    // We cannot replace this because we do not know
    // if the two slots are different.
    mstore(0, sload(x))
}
// ====
// EVMVersion: >=shanghai
// ----
// step: loadResolver
//
// {
//     {
//         let x := calldataload(1)
//         sstore(x, 7)
//         sstore(calldataload(0), 6)
//         mstore(0, sload(x))
//     }
// }
