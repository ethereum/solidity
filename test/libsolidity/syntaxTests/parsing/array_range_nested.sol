pragma experimental ABIEncoderV2;
contract C {
    function f(uint256[][] calldata x) external pure {
        x[0][1:2];
        x[1:2][1:2];
        uint256 a = x[1:2][1:2][1:][3:][0][2];
        uint256 b = x[1:][3:4][1][1:][2:3][0];
        a; b;
    }
}
// ----
// Warning: (0-33): Experimental features are turned on. Do not use experimental features on live deployments.
