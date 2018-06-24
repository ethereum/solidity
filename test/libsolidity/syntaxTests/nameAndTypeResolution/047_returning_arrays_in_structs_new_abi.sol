pragma experimental ABIEncoderV2;

contract C {
    struct S { string[] s; }
    function f() public pure returns (S) {}
}
// ----
// Warning: (0-33): Experimental features are turned on. Do not use experimental features on live deployments.
