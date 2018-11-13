pragma experimental ABIEncoderV2;
contract C {
    struct S { uint x; }
    function f() public pure {
        S[] memory s;
        abi.encode(s);
    }
}
// ----
// Warning: (0-33): Experimental features are turned on. Do not use experimental features on live deployments.
