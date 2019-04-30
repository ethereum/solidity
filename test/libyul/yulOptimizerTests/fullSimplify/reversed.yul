{
    let a := add(0, mload(0))
    mstore(0, a)
}
// ====
// step: fullSimplify
// ----
// {
//     let _1 := 0
//     mstore(_1, mload(_1))
// }
