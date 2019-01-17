{
    let a := mload(sub(7, 7))
    let b := sub(a, 0)
}
// ----
// expressionSimplifier
// {
//     let a := mload(0)
//     let b := a
// }
