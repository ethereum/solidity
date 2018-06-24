contract C {
    struct S { uint a; bool x; }
    function f() public {
        S memory s = S({a: 1, x: true});
    }
}
// ----
// Warning: (80-90): Unused local variable.
// Warning: (50-118): Function state mutability can be restricted to pure
