pragma experimental ABIEncoderV2;
contract Test {
    function f(uint[3][4] calldata) external { }
}
// ----
// Warning: (0-33): Experimental features are turned on. Do not use experimental features on live deployments.
