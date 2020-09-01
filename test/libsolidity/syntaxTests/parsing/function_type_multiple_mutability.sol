contract C {
  function f() public pure {
    function() pure pure g;
  }
}
// ----
// ParserError 9680: (62-66): State mutability already specified as "pure".
