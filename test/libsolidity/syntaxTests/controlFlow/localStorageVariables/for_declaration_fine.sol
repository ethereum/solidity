contract C {
    struct S { bool f; }
    S s;
    function f() internal view {
        S storage c;
        for(c = s;;) {
        }
        c;
    }
    function g() internal view {
        S storage c;
        for(; (c = s).f;) {
        }
        c;
    }
}
// ----
