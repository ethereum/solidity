contract C {
    struct S { bool f; }
    S s;
    function f() internal view returns (S storage c) {
        for(c = s;;) {
        }
    }
    function g() internal view returns (S storage c) {
        for(; (c = s).f;) {
        }
    }
}
// ----
