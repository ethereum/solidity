contract C {
  function f() public {
    bytes32 b;
    b[-1];
  }
}
// ----
// TypeError 7407: (58-60): Type int_const -1 is not implicitly convertible to expected type uint256. Cannot implicitly convert signed literal to unsigned type.
// TypeError 6318: (56-61): Index expression cannot be represented as an unsigned integer.
