contract C {
  uint256 Test;

  function f() {
    emit Test();
  }
}
// ----
// TypeError: (56-62): Type is not callable
// TypeError: (56-60): Expression has to be an event invocation.
