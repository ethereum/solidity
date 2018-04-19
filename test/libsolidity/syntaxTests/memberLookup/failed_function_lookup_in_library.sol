library L {
  function f(uint, uint) {}
  function f(uint) {}
}
contract C {
  function g() { L.f(1, 2, 3); }
}
// ----
// TypeError: (94-97): Member "f" not found or not visible after argument-dependent lookup in type(library L)
