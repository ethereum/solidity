contract C {
    struct S { bool f; }
    S s;
    function f() internal pure {}
    function g() internal view returns (S storage) { return s; }
    function h() internal view returns (S storage c) { return s; }
    function i() internal view returns (S storage c) { c = s; }
    function j() internal view returns (S storage c) { (c) = s; }
}
// ----
