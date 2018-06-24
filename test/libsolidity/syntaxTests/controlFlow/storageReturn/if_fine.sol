contract C {
    struct S { bool f; }
    S s;
    function f(bool flag) internal view returns (S storage c) {
        if (flag) c = s;
        else c = s;
    }
    function g(bool flag) internal view returns (S storage c) {
        if (flag) c = s;
        else { c = s; }
    }
    function h(bool flag) internal view returns (S storage c) {
        if (flag) c = s;
        else
        {
            if (!flag) c = s;
            else c = s;
        }
    }
    function i() internal view returns (S storage c) {
        if ((c = s).f) {
        }
    }
    function j() internal view returns (S storage c) {
        if ((c = s).f && !(c = s).f) {
        }
    }
}
// ----
