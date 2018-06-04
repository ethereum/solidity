pragma experimental ABIEncoderV2;

contract C {
    struct S1 { }
    struct S2 { }
    function f(S1) pure {}
    function f(S2) pure {}
}
// ----
// Warning: (0-33): Experimental features are turned on. Do not use experimental features on live deployments.
// Warning: (52-65): Defining empty structs is deprecated.
// Warning: (70-83): Defining empty structs is deprecated.
// TypeError: (115-137): Function overload clash during conversion to external types for arguments.
