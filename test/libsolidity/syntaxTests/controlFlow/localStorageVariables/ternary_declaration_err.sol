contract C {
    struct S { bool f; }
    S s;
    function f(bool flag) internal view {
        S storage c;
        flag ? (c = s).f : false;
        c;
    }
    function g(bool flag) internal view {
        S storage c;
        flag ? false : (c = s).f;
        c;
    }
}
// ----
// TypeError: (152-153): This variable is of storage pointer type and can be accessed without prior assignment, which would lead to undefined behaviour.
// TypeError: (266-267): This variable is of storage pointer type and can be accessed without prior assignment, which would lead to undefined behaviour.
