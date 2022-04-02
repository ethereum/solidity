function f() {}
contract C {
  function f() public {}
  function g() public {
    f();
  }
}
// ----
// Warning 2519: (31-53='function f() public {}'): This declaration shadows an existing declaration.
