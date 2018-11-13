{
    let a := mload(0)
    let b := sub(a, a)
}
// ----
// expressionSimplifier
// {
//     let a := mload(0)
//     let b := 0
// }
