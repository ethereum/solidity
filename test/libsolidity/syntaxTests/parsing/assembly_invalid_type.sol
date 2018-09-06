contract C {
  function f() public pure {
    assembly "failasm" {}
  }
}
// ----
// ParserError: (55-64): Only "evmasm" supported.
