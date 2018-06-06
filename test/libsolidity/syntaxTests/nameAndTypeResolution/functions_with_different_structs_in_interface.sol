pragma experimental ABIEncoderV2;

contract C {
    struct S1 { function() external a; }
    struct S2 { bytes24 a; }
    function f(S1) pure {}
    function f(S2) pure {}
}
// ----
// Warning: (0-33): Experimental features are turned on. Do not use experimental features on live deployments.
// Warning: (122-144): No visibility specified. Defaulting to "public". 
// Warning: (149-171): No visibility specified. Defaulting to "public". 
