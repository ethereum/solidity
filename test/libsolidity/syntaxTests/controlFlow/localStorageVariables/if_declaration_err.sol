contract C {
    struct S { bool f; }
    S s;
    function f(bool flag) internal {
        S storage c;
        if (flag) c = s;
        c;
    }
    function g(bool flag) internal {
        S storage c;
        if (flag) c = s;
        else
        {
            if (!flag) c = s;
            else s.f = true;
        }
        c;
    }
}
// ----
// TypeError 3464: (138-139='c'): This variable is of storage pointer type and can be accessed without prior assignment, which would lead to undefined behaviour.
// TypeError 3464: (330-331='c'): This variable is of storage pointer type and can be accessed without prior assignment, which would lead to undefined behaviour.
