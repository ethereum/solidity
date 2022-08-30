{
    mstore(0, mod(mul(mload(0), mload(1)), 32))
}
// ----
// step: expressionSimplifier
//
// {
//     {
//         let _3 := mload(1)
//         let _4 := 0
//         mstore(_4, mulmod(mload(_4), _3, 32))
//     }
// }
