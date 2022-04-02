{
  function f() -> a, b {}
  let x, x := f()
}
// ----
// DeclarationError 1395: (30-45='let x, x := f()'): Variable name x already taken in this scope.
