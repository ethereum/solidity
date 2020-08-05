contract C {
    struct S { bool f; }
    S s;
    function f() internal pure {}
    function g() internal view { s; }
    function h() internal view {
        S storage c;
        c = s;
        c;
    }
    function i() internal view {
        S storage c;
        (c) = s;
        c;
    }
}
// ----
