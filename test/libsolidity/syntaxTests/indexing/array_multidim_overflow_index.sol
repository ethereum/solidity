contract C {
  function f() public {
    bytes[32] memory a;
    a[8**90][8**90][1 - 8**90];
  }
}
// ----
// TypeError: (67-72): Type int_const 1897...(74 digits omitted)...1424 is not implicitly convertible to expected type uint256.
// TypeError: (74-79): Type int_const 1897...(74 digits omitted)...1424 is not implicitly convertible to expected type uint256.
// TypeError: (81-90): Type int_const -189...(75 digits omitted)...1423 is not implicitly convertible to expected type uint256.
// TypeError: (65-91): Index expression cannot be represented as an unsigned integer.
