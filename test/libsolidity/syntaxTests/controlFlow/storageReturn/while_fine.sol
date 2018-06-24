contract C {
    struct S { bool f; }
    S s;
    function f() internal view returns (S storage c) {
        while((c = s).f) {
        }
    }
    function g() internal view returns (S storage c) {
        c = s;
        while(false) {
        }
    }
    function h() internal view returns (S storage c) {
        while(false) {
        }
        c = s;
    }
}
// ----
