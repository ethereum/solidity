contract C {
    struct S { bool f; }
    S s;
    function f() internal view {
        S storage c;
        (c = s).f && false;
        c;
    }
    function g() internal view {
        S storage c;
        (c = s).f || true;
        c;
    }
}
// ----
