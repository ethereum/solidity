{
  function f(x) {}
  let a
  f(a)
  a := 4
}
// ----
// varDeclPropagator
// {
//     function f(x)
//     {
//     }
//     let a
//     f(a)
//     a := 4
// }
