contract C {
  function f() public pure {
    assembly "failasm" {}
  }
}
// ----
// ParserError: (55-64): Only "evmasm" supported.
// ParserError: (55-64): In <Statement>, ';' is expected; got 'StringLiteral' instead.
// ParserError: (72-73): Expected pragma, import directive or contract/interface/library definition.
