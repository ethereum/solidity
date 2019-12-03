{
  function f() -> x { x := g() }
  function g() -> x { for {} 1 {} {} }
  pop(f())
}
// ----
// :
// f:
// g:
