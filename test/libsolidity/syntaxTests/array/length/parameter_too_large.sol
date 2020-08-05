contract C {
  function f(bytes32[1263941234127518272] memory) public pure {}
}
// ----
// TypeError 1534: (26-61): Type too large for memory.
