contract C {
    struct S {
        uint[] a;
    }
    S s;
    function f() public {
        s.a.length = 4;
    }
}
// ----
// TypeError: (95-105): Member "length" is read-only and cannot be used to resize arrays.
