contract C {
  function f(bytes32[1263941234127518272][500] memory) public pure {}
  function f(uint[2**30][] memory) public pure {}
  function f(uint[2**30][2**30][] memory) public pure {}
  function f(uint[2**16][2**16][] memory) public pure {}
}
// ----
// TypeError 1534: (26-66): Type too large for memory.
// TypeError 1534: (96-116): Type too large for memory.
// TypeError 1534: (146-173): Type too large for memory.
// TypeError 1534: (203-230): Type too large for memory.
