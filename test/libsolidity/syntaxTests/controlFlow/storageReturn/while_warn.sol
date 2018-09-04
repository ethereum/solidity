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
// Warning: (87-98): This variable is of storage pointer type and might be returned without assignment. This can cause storage corruption. Assign the variable (potentially from itself) to remove this warning.
