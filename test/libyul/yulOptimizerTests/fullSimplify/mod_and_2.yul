{
    mstore(0, mod(calldataload(0), exp(2, 255)))
}
// ====
// EVMVersion: >=shanghai
// ----
// step: fullSimplify
//
// {
//     {
//         mstore(0, and(calldataload(0), 0x7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff))
//     }
// }
