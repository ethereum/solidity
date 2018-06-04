pragma experimental ABIEncoderV2;

contract C {
    struct S { function() internal a; }
    function f(S) {}
}
// ----
// Warning: (0-33): Experimental features are turned on. Do not use experimental features on live deployments.
// TypeError: (103-104): Internal or recursive type is not allowed for public or external functions.
