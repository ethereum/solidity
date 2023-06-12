{
    let x := calldataload(1)
    sstore(x, 7)
    sstore(calldataload(0), 7)
    // We can replace this because both values that were
    // written are 7.
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
//         let _2 := 7
//         sstore(x, _2)
//         sstore(calldataload(0), _2)
//         mstore(0, _2)
//     }
// }
