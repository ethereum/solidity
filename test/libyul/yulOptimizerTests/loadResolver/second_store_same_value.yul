{
    let x := calldataload(1)
    sstore(x, 7)
    sstore(calldataload(0), 7)
    // We can replace this because both values that were
    // written are 7.
    mstore(0, sload(x))
}
// ----
// step: loadResolver
//
// {
//     {
//         let x := calldataload(1)
//         let _1 := 7
//         sstore(x, _1)
//         let _2 := 0
//         sstore(calldataload(_2), _1)
//         mstore(_2, _1)
//     }
// }
