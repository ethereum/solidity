{
    let x := calldataload(1)
    sstore(x, 7)
    sstore(calldataload(0), 6)
    // We cannot replace this because we do not know
    // if the two slots are different.
    mstore(0, sload(x))
}
// ====
// step: loadResolver
// ----
// {
//     let x := calldataload(1)
//     sstore(x, 7)
//     let _3 := 6
//     let _4 := 0
//     sstore(calldataload(_4), _3)
//     mstore(_4, sload(x))
// }
