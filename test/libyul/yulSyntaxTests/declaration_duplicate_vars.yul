{
  function f() -> a, b {}
  let x, x := f()
}
// ----
// DeclarationError 1395: (30-45): Variable name x already taken in this scope.
