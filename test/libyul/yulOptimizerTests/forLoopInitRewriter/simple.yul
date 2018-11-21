{
  let random := 42
  for { let a := 1 } iszero(eq(a, 10)) { a := add(a, 1) } {
    a := add(a, 1)
  }
}
// ----
// forLoopInitRewriter
// {
//     let random := 42
//     let a := 1
//     for {
//     }
//     iszero(eq(a, 10))
//     {
//         a := add(a, 1)
//     }
//     {
//         a := add(a, 1)
//     }
// }
