contract C {
    struct S { bool f; }
    S s;
    function f() internal view returns (S storage c) {
        (c = s).f && false;
    }
    function g() internal view returns (S storage c) {
        (c = s).f || true;
    }
}
// ----
