contract C {
  function f() public pure {
    assembly {
      leave
    }
  }
}
// ----
// SyntaxError: (63-68): Keyword "leave" can only be used inside a function.
