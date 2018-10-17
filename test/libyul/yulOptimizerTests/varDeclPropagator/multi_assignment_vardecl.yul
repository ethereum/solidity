{
  function f() -> a, b, c {}
  let x, y, z 
  z, x, y := f()
}
// ----
// varDeclPropagator
// {
//     function f() -> a, b, c
//     {
//     }
//     let z, x, y := f()
// }
