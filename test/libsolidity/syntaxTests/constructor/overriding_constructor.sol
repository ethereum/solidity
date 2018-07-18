contract A { function f() public {} }
contract B is A {
  function A() public pure returns (uint8) {}
  function g() public {
    A.f();
  }
}
// ----
// Warning: (58-101): This declaration shadows an existing declaration.
// TypeError: (130-133): Member "f" not found or not visible after argument-dependent lookup in function () pure returns (uint8).
