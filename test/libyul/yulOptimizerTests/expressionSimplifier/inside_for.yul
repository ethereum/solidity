{
    let a := 10
    for {  } iszero(eq(a, 0)) { a := add(a, 1) } {}
}
// ====
// step: expressionSimplifier
// ----
// {
//     let a := 10
//     for {
//     }
//     iszero(iszero(a))
//     {
//         a := add(a, 1)
//     }
//     {
//     }
// }
