contract C {
    struct S { bool f; }
    S s;
    function f(bool flag) internal view {
        S storage c;
        if (flag) c = s;
        else c = s;
        c;
    }
    function g(bool flag) internal view {
        S storage c;
        if (flag) c = s;
        else { c = s; }
        c;
    }
    function h(bool flag) internal view {
        S storage c;
        if (flag) c = s;
        else
        {
            if (!flag) c = s;
            else c = s;
        }
        c;
    }
    function i() internal view {
        S storage c;
        if ((c = s).f) {
        }
        c;
    }
    function j() internal view {
        S storage c;
        if ((c = s).f && !(c = s).f) {
        }
        c;
    }
}
// ----
