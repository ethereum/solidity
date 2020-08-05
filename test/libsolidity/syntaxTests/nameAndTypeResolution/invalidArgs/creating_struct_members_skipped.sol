contract C {
    struct S { uint a; uint b; mapping(uint=>uint) c; }

    function f() public {
        S({a: 1});
    }
}
// ----
// TypeError 9515: (104-113): Struct containing a (nested) mapping cannot be constructed.
