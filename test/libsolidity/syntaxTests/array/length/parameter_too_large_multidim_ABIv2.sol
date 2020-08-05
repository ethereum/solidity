pragma experimental ABIEncoderV2;

contract C {
  function f(bytes32[1263941234127518272][500] memory) public pure {}
  function f(uint[2**30][2**30][][] memory) public pure {}
}
// ----
// TypeError 1534: (61-101): Type too large for memory.
// TypeError 1534: (131-160): Type too large for memory.
