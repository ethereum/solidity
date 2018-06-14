pragma experimental ABIEncoderV2;

contract C {
    struct S1 { int i; }
    struct S2 { int i; }
    function f(S1) pure {}
    function f(S2) pure {}
}
// ----
// Warning: (0-33): Experimental features are turned on. Do not use experimental features on live deployments.
// TypeError: (129-151): Function overload clash during conversion to external types for arguments.
