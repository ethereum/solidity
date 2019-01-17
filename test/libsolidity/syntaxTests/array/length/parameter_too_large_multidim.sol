contract C {
  function f(bytes32[1263941234127518272][500] memory) public pure {}
  function f(uint[2**30][] memory) public pure {}
  function f(uint[2**30][2**30][] memory) public pure {}
  function f(uint[2**16][2**16][] memory) public pure {}
}
// ----
// TypeError: (26-66): Array is too large to be encoded.
// TypeError: (96-116): Array is too large to be encoded.
// TypeError: (146-173): Array is too large to be encoded.
// TypeError: (203-230): Array is too large to be encoded.
