{
    sstore(calldataload(0), calldataload(10))
    let t := sload(calldataload(10))
    let q := sload(calldataload(0))
    mstore(t, q)
}
// ----
// step: loadResolver
//
// {
//     {
//         let _1 := calldataload(10)
//         sstore(calldataload(0), _1)
//         mstore(sload(_1), _1)
//     }
// }
