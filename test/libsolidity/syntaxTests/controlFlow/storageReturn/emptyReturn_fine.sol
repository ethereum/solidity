contract C {
    struct S { bool f; }
    S s;
    function f() internal view returns (S storage c, S storage d) { c = s; d = s; return; }
}
// ----
