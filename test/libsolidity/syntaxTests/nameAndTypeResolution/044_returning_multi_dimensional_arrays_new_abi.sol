pragma experimental ABIEncoderV2;

contract C {
    function f() public pure returns (string[][] memory) {}
}
// ----
// Warning: (0-33): Experimental features are turned on. Do not use experimental features on live deployments.
