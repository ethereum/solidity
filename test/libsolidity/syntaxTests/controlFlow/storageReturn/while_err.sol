contract C {
    struct S { bool f; }
    S s;
    function f() internal view returns (S storage c) {
        while(false) {
            c = s;
        }
    }
}
// ----
// TypeError: (87-98): This variable is of storage pointer type and can be returned without prior assignment, which would lead to undefined behaviour.
