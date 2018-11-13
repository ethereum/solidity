contract C {
  uint256 Test;

  function f() public {
    emit Test();
  }
}
// ----
// TypeError: (63-69): Type is not callable
// TypeError: (63-67): Expression has to be an event invocation.
