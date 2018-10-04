contract C {
  function f(bytes32[1263941234127518272][500] memory) public pure {}
}
// ----
// TypeError: (26-66): Array is too large to be encoded.
