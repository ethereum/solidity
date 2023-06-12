{
    sstore(calldataload(0), calldataload(10))
    let t := sload(calldataload(10))
    let q := sload(calldataload(0))
    mstore(t, q)
}
// ====
// EVMVersion: >=shanghai
// ----
// step: loadResolver
//
// {
//     {
//         let _2 := calldataload(10)
//         sstore(calldataload(0), _2)
//         let t := sload(_2)
//         mstore(t, sload(calldataload(0)))
//     }
// }
