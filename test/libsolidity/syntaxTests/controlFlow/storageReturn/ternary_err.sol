contract C {
    struct S { bool f; }
    S s;
    function f(bool flag) internal view returns (S storage c) {
        flag ? (c = s).f : false;
    }
    function g(bool flag) internal view returns (S storage c) {
        flag ? false : (c = s).f;
    }
}
// ----
// TypeError: (96-107): This variable is of storage pointer type and can be returned without prior assignment.
// TypeError: (200-211): This variable is of storage pointer type and can be returned without prior assignment.
