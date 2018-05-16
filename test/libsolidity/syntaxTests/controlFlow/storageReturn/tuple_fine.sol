contract C {
    struct S { bool f; }
    S s;
    function f() internal view returns (S storage, uint) {
        return (s,2);
    }
    function g() internal view returns (S storage c) {
        uint a;
        (c, a) = f();
    }
}
// ----
