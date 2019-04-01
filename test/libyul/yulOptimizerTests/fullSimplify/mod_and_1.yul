{
    mstore(0, mod(calldataload(0), exp(2, 8)))
}
// ====
// step: fullSimplify
// ----
// {
//     let _4 := 0
//     mstore(_4, and(calldataload(_4), 255))
// }
