contract C {
    struct S { bool f; }
    S s;
    function f() internal pure returns (S storage) {
        assembly {
        }
    }
}
// ----
// TypeError: (87-88): This variable is of storage pointer type and might be returned without assignment and could be used uninitialized. Assign the variable (potentially from itself) to fix this error.
