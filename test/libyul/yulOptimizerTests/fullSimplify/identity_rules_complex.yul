{
    let a := sub(calldataload(0), calldataload(0))
    mstore(a, 0)
}
// ====
// step: fullSimplify
// ----
// {
//     mstore(0, 0)
// }
