{
    mstore(0, mod(mul(mload(0), mload(1)), 32))
}
// ====
// EVMVersion: >=shanghai
// ----
// step: expressionSimplifier
//
// {
//     {
//         mstore(0, mulmod(mload(0), mload(1), 32))
//     }
// }
