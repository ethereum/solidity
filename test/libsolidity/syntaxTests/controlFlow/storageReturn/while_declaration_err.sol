contract C {
    struct S { bool f; }
    S s;
    function f() internal view {
        S storage c;
        while(false) {
            c = s;
        }
        c;
    }
}
// ----
// TypeError: (161-162): This variable is of storage pointer type and can be accessed without prior assignment, which would lead to undefined behaviour.
