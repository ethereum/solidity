contract C {
  function f() public {
    bytes32 b;
    b[888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888888];
  }
}
// ----
// TypeError: (58-169): Type int_const 8888...(103 digits omitted)...8888 is not implicitly convertible to expected type uint256.
// TypeError: (56-170): Index expression cannot be represented as an unsigned integer.
