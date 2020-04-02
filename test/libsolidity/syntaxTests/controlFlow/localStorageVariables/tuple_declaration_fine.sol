contract C {
    struct S { bool f; }
    S s;
    function f() internal view returns (S storage, uint) {
      return (s,2);
    }
    function g() internal view {
        uint a;
        S storage c;
        (c, a) = f();
        c;
    }
    function h() internal view {
        (s, s);
    }
}
// ----
