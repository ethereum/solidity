contract C {
  function f() public pure
  {
    bytes32[1263941234127518272][500] memory x;
    uint[2**30][] memory y;
    uint[2**30][2**30][] memory z;
    uint[2**16][2**16][] memory w;
  }
}
// ----
// TypeError 1534: (48-90): Type too large for memory.
// TypeError 1534: (96-118): Type too large for memory.
// TypeError 1534: (124-153): Type too large for memory.
// TypeError 1534: (159-188): Type too large for memory.
