contract C {
  function f() public {
    bytes[32] memory a;
    a[-1];
  }
}
// ----
// TypeError: (67-69): Type int_const -1 is not implicitly convertible to expected type uint256.
