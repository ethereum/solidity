pragma experimental ABIEncoderV2;
contract C {
    struct S { uint256 x; }
    function f(S calldata s) external pure {
        s.x = 42;
    }
}
// ----
// Warning: (0-33): Experimental features are turned on. Do not use experimental features on live deployments.
// TypeError: (128-131): Calldata structs are read-only.
