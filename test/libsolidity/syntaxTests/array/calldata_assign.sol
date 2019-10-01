pragma experimental ABIEncoderV2;
contract Test {
    function f(uint256[] calldata s) external { s[0] = 4; }
}
// ----
// Warning: (0-33): Experimental features are turned on. Do not use experimental features on live deployments.
// TypeError: (98-102): Calldata arrays are read-only.
