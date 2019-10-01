contract C {
    struct S { bool f; }
    S s;
    function f() internal pure returns (S storage) {
        assembly {
        }
    }
}
// ----
// TypeError: (87-96): This variable is of storage pointer type and can be returned without prior assignment, which would lead to undefined behaviour.
