pragma experimental ABIEncoderV2;
library c {
    struct S { uint x; }
    function f() public returns (S memory) {}
}
// ----
// Warning: (0-33): Experimental features are turned on. Do not use experimental features on live deployments.
// Warning: (75-116): Function state mutability can be restricted to pure
