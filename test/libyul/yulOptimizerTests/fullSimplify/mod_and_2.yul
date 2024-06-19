{
    mstore(0, mod(calldataload(0), exp(2, 255)))
}
// ----
// step: fullSimplify
//
// {
//     {
//         let _1 := 0
//         mstore(_1, and(calldataload(_1), 0x7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff))
//     }
// }
