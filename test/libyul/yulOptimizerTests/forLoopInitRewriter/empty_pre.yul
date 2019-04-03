{
  let a := 1
  for {} iszero(eq(a, 10)) { a := add(a, 1) } {
    a := add(a, 1)
  }
}
// ====
// step: forLoopInitRewriter
// ----
// {
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
