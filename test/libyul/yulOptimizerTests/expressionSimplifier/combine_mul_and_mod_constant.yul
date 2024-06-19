{
    mstore(0, mod(mul(mload(0), mload(1)), 32))
}
// ----
// step: expressionSimplifier
//
// {
//     {
//         let _1 := mload(1)
//         let _2 := 0
//         mstore(_2, mulmod(mload(_2), _1, 32))
//     }
// }
