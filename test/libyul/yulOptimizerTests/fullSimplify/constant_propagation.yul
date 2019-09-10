{
    let a := add(7, sub(mload(0), 7))
    mstore(a, 0)
}
// ====
// step: fullSimplify
// ----
// {
//     let _2 := 0
//     mstore(mload(_2), _2)
// }
