{
  let a := 1
  for { } 42 { } { }
  for { } "hello" { } { }
  for { } 0 { } { }
  for { } a { } { }
  for { } add(a, a) { } { }
}
// ====
// step: forLoopConditionIntoBody
// ----
// {
//     let a := 1
//     for { } 42 { }
//     { }
//     for { } "hello" { }
//     { }
//     for { } 0 { }
//     { }
//     for { } a { }
//     { }
//     for { } 1 { }
//     {
//         if iszero(add(a, a)) { break }
//     }
// }
