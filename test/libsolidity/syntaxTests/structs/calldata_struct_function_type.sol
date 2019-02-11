pragma experimental ABIEncoderV2;
contract C {
    struct S { function (uint) external returns (uint) fn; }
    function f(S calldata s) external returns (uint256 a) {
        return s.fn(42);
    }
}
// ----
// Warning: (0-33): Experimental features are turned on. Do not use experimental features on live deployments.
