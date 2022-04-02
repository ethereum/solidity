contract C {
  function f() public pure {
    assembly "failasm" {}
  }
}
// ----
// ParserError 4531: (55-64='"failasm"'): Only "evmasm" supported.
