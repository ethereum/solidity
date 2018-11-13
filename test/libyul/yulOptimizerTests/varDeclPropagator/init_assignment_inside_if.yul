{
  let a := 4
  let x
  if a {
    x := 2
  }
}
// ----
// varDeclPropagator
// {
//     let a := 4
//     let x
//     if a
//     {
//         x := 2
//     }
// }
