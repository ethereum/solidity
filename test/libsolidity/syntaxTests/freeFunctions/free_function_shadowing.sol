function f() {}
contract C {
  function f() public {}
  function g() public {
    f();
  }
}
// ----
// Warning 2519: (31-53): This declaration shadows an existing declaration.
