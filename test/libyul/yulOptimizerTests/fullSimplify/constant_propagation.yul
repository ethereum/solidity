{
    let a := add(7, sub(mload(0), 7))
    mstore(a, 0)
}
// ----
// step: fullSimplify
//
// {
//     {
//         let _1 := 0
//         mstore(mload(_1), _1)
//     }
// }
