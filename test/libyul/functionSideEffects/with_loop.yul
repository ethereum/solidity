{
  function f() -> x { x := g() }
  function g() -> x { for {} 1 {} {} }
  pop(f())
}
// ----
// : movable apart from effects, can loop
// f: movable apart from effects, can loop
// g: movable apart from effects, can loop
