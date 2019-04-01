{
    for { let a := 10 } iszero(eq(a, 0)) { a := add(a, 1) } {}
}
// ====
// step: expressionSimplifier
// ----
// {
//     for {
//         let a := 10
//     }
//     iszero(iszero(a))
//     {
//         a := add(a, 1)
//     }
//     {
//     }
// }
