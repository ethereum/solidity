{
    for { let i := 0 } lt(i, 4) { i := add(i, 1) } {
        mstore(i, 0)
    }

    for { let i := 0 } lt(i, 4) { i := add(i, 1) } {
        sstore(i, 0)
        sstore(0, call(0, 0, 0, 0, 0, 0, 0))
    }
}
// ----
// step: zeroByReturndatasizeReplacer
//
// {
//     for { let i := returndatasize() } lt(i, 4) { i := add(i, 1) }
//     { mstore(i, returndatasize()) }
//     for { let i_1 := 0 } lt(i_1, 4) { i_1 := add(i_1, 1) }
//     {
//         sstore(i_1, 0)
//         sstore(0, call(0, 0, 0, 0, 0, 0, 0))
//     }
// }
