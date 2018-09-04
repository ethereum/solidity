contract C {
    struct S { bool f; }
    S s;
    function f(bool flag) internal view returns (S storage c) {
        flag ? c = s : c = s;
    }
    function g(bool flag) internal view returns (S storage c) {
        flag ? c = s : (c = s);
    }
    function h(bool flag) internal view returns (S storage c) {
        flag ? (c = s).f : (c = s).f;
    }
}
// ----
