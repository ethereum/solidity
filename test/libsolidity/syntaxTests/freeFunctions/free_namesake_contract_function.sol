function f() pure returns (uint) { return 1337; }
contract C {
  function f() public pure returns (uint) {
    return f();
  }
}
// ----
// Warning 2519: (65-126): This declaration shadows an existing declaration.
