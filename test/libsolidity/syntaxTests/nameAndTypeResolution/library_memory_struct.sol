pragma experimental ABIEncoderV2;
library c {
    struct S { uint x; }
    function f() public returns (S ) {}
}
// ----
// Warning: (0-33): Experimental features are turned on. Do not use experimental features on live deployments.
// Warning: (75-110): Function state mutability can be restricted to pure
