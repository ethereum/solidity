pragma experimental ABIEncoderV2;
contract Test {
    function f(uint[][] calldata) external { }
}
// ----
// Warning: (0-33): Experimental features are turned on. Do not use experimental features on live deployments.
// TypeError: (65-82): Calldata arrays with dynamically encoded base types are not yet supported.
