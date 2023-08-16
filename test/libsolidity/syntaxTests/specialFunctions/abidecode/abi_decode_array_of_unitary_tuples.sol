contract C {
    function f() pure public {
        abi.decode("", ((uint)[2]));
        abi.decode("", ((uint)[]));
        abi.decode("", ((uint)[][3]));
        abi.decode("", ((uint)[4][]));
        abi.decode("", ((uint)[5][6]));
        abi.decode("", (((uint))[5][6]));
    }
}
// ----
// Warning 6133: (52-79): Statement has no effect.
// Warning 6133: (89-115): Statement has no effect.
// Warning 6133: (125-154): Statement has no effect.
// Warning 6133: (164-193): Statement has no effect.
// Warning 6133: (203-233): Statement has no effect.
// Warning 6133: (243-275): Statement has no effect.
