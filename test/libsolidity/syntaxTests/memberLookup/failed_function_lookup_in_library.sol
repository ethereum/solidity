library L {
  function f(uint, uint) public {}
  function f(uint) public {}
}
contract C {
  function g() public { L.f(1, 2, 3); }
}
// ----
// TypeError 9582: (115-118): Member "f" not found or not visible after argument-dependent lookup in type(library L).
