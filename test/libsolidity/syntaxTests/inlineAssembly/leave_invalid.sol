contract C {
  function f() public pure {
    assembly {
      leave
    }
  }
}
// ----
// SyntaxError 8149: (63-68='leave'): Keyword "leave" can only be used inside a function.
