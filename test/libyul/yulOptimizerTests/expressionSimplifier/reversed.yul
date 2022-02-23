{
    let a := add(0, mload(0))
    sstore(0, a)
}
// ----
// step: expressionSimplifier
//
// {
//     {
//         let _1 := 0
//         sstore(_1, mload(_1))
//     }
// }
