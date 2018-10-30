contract C {
  function f(bytes32[1263941234127518272] memory) public pure {}
}
// ----
// TypeError: (26-61): Array is too large to be encoded.
