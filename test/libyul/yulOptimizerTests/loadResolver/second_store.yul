{
    let x := calldataload(1)
    sstore(x, 7)
    sstore(calldataload(0), 6)
    // We cannot replace this because we do not know
    // if the two slots are different.
    mstore(0, sload(x))
}
// ----
// step: loadResolver
//
// {
//     {
//         let x := calldataload(1)
//         sstore(x, 7)
//         let _1 := 6
//         let _2 := 0
//         sstore(calldataload(_2), _1)
//         mstore(_2, sload(x))
//     }
// }
