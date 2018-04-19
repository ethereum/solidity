contract C {
  function f(uint, uint) {}
  function f(uint) {}
  function g() { f(1, 2, 3); }
}
// ----
// TypeError: (80-81): No matching declaration found after argument-dependent lookup.
