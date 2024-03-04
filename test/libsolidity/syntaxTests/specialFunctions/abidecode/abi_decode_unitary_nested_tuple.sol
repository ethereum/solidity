struct S { int a; }

contract C {
    function f() pure public {
        abi.decode("", (((uint))));
        abi.decode("", ((((uint)))));
        abi.decode("", (((S))));
    }
}
// ----
// Warning 6133: (73-99): Statement has no effect.
// Warning 6133: (109-137): Statement has no effect.
// Warning 6133: (147-170): Statement has no effect.
