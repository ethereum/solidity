contract C {
    struct S { uint a; bool x; }
    function f() public {
        S memory s = S(1, true);
    }
}
// ----
// Warning 2072: (80-90): Unused local variable.
// Warning 2018: (50-110): Function state mutability can be restricted to pure
