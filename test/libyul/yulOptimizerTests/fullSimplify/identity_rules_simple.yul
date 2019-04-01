{
    let a := mload(0)
    mstore(0, sub(a, a))
}
// ====
// step: fullSimplify
// ----
// {
//     let _1 := 0
//     pop(mload(_1))
//     mstore(_1, 0)
// }
