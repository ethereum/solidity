contract C {
  function f() public {
    emit;
  }
}
// ----
// ParserError 5620: (45-46): Expected event name or path.
