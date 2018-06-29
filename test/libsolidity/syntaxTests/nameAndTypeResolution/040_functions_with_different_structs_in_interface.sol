pragma experimental ABIEncoderV2;

contract C {
    struct S1 { function() external a; }
    struct S2 { bytes24 a; }
    function f(S1) public pure {}
    function f(S2) public pure {}
}
// ----
// Warning: (0-33): Experimental features are turned on. Do not use experimental features on live deployments.
