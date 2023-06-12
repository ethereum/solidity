{
    mstore(0, mod(add(mload(0), mload(1)), 32))
}
// ====
// EVMVersion: >=shanghai
// ----
// step: expressionSimplifier
//
// {
//     {
//         mstore(0, addmod(mload(0), mload(1), 32))
//     }
// }
