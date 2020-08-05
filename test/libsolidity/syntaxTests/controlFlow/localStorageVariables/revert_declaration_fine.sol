contract C {
    struct S { bool f; }
    S s;
    function g(bool flag) internal view {
        S storage c;
        if (flag) c = s;
        else revert();
        s;
    }
}
// ----
