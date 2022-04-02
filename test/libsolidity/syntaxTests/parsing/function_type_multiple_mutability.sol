contract C {
  function f() public pure {
    function() pure pure g;
  }
}
// ----
// ParserError 9680: (62-66='pure'): State mutability already specified as "pure".
