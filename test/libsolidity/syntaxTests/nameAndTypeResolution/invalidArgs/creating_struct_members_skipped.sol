contract C {
    struct S { uint a; uint b; mapping(uint=>uint) c; }

    function f() public {
        S memory s = S({a: 1});
    }
}
// ----
// TypeError 9515: (117-126): Struct containing a (nested) mapping cannot be constructed.
