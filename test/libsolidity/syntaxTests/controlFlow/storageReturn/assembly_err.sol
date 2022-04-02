contract C {
    struct S { bool f; }
    S s;
    function f() internal pure returns (S storage) {
        assembly {
        }
    }
}
// ----
// TypeError 3464: (87-96='S storage'): This variable is of storage pointer type and can be returned without prior assignment, which would lead to undefined behaviour.
