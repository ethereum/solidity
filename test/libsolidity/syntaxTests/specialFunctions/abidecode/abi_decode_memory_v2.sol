pragma experimental "ABIEncoderV2";

contract C {
  struct S { uint x; uint[] b; }
  function f() public pure returns (S memory, bytes memory, uint[][2] memory) {
    return abi.decode("abc", (S, bytes, uint[][2]));
  }
}
// ----
// Warning: (0-35): Experimental features are turned on. Do not use experimental features on live deployments.
