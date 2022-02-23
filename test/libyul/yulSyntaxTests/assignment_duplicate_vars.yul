{
  function f() -> a, b {}
  function g() -> a, b, c {}
  let x, y
  x, x := f()
  y, x, y := g()
}
// ----
// DeclarationError 9005: (70-81): Variable x occurs multiple times on the left-hand side of the assignment.
// DeclarationError 9005: (84-98): Variable y occurs multiple times on the left-hand side of the assignment.
