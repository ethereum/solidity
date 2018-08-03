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
// TypeError: (87-98): This variable is of storage pointer type and might be returned without assignment and could be used uninitialized. Assign the variable (potentially from itself) to fix this error.
