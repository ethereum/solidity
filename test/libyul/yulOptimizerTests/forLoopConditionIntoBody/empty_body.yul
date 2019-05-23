{
  for { let a := 1 } iszero(eq(a, 10)) { a := add(a, 1) } { }
}
// ====
// step: forLoopConditionIntoBody
// ----
// {
//     for { let a := 1 } 1 { a := add(a, 1) }
//     {
//         if iszero(iszero(eq(a, 10))) { break }
//     }
// }
