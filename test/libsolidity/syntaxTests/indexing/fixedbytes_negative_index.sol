contract C {
  function f() public {
    bytes32 b;
    b[-1];
  }
}
// ----
// TypeError: (58-60): Type int_const -1 is not implicitly convertible to expected type uint256.
// TypeError: (56-61): Index expression cannot be represented as an unsigned integer.
