{
  function f() -> a, b {}
  function g() -> a, b, c {}
  let x, y
  x, x := f()
  y, x, y := g()
}
// ----
// DeclarationError 9005: (70-81='x, x := f()'): Variable x occurs multiple times on the left-hand side of the assignment.
// DeclarationError 9005: (84-98='y, x, y := g()'): Variable y occurs multiple times on the left-hand side of the assignment.
