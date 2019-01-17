pragma experimental ABIEncoderV2;

contract C {
  function f(bytes32[1263941234127518272][500] memory) public pure {}
  function f(uint[2**30][2**30][][] memory) public pure {}
}
// ----
// Warning: (0-33): Experimental features are turned on. Do not use experimental features on live deployments.
// TypeError: (61-101): Array is too large to be encoded.
// TypeError: (131-160): Array is too large to be encoded.
