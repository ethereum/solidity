contract C {
  function f() public pure {
    assembly {
      function g() -> a,b, c {}
      let a, sub, mov := g()
    }
  }
}
// ----
// ParserError: (102-105): Cannot use instruction names for identifier names.
// ParserError: (105-106): Expected ';' but got ','
