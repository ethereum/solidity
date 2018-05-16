contract C {
    struct S { bool f; }
    S s;
    function f() internal view returns (S c) {
        c = s;
    }
    function g() internal view returns (S) {
        return s;
    }
    function h() internal pure returns (S) {
    }
    function i(bool flag) internal view returns (S c) {
        if (flag) c = s;
    }
    function j(bool flag) internal view returns (S) {
        if (flag) return s;
    }
}
// ----
