{
    mstore(0, mod(calldataload(0), exp(2, 255)))
}
// ----
// fullSimplify
// {
//     let _4 := 0
//     mstore(_4, and(calldataload(_4), 0x7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff))
// }
