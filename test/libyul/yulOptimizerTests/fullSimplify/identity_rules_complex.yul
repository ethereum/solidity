{
    let a := sub(calldataload(0), calldataload(0))
    mstore(a, 0)
}
// ====
// EVMVersion: >=shanghai
// ----
// step: fullSimplify
//
// {
//     {
//         mstore(sub(calldataload(0), calldataload(0)), 0)
//     }
// }
