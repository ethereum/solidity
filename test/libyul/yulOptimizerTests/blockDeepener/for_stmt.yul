{
    for { let a := 1 } iszero(eq(a, 10)) { a := add(a, 1) }
    { a := add(a, 1) a := add(a,1) }
}
// ====
// step: blockDeepener
// ----
// {
//     for { let a := 1 } iszero(eq(a, 10)) { a := add(a, 1) }
//     {
//         a := add(a, 1)
//         { a := add(a, 1) }
//     }
// }
