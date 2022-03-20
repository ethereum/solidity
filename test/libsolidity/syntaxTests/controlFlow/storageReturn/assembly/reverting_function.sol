contract C {
    struct S { bool f; }
    S s;
    function f() internal pure returns (S storage c) {
        // this could be allowed, but currently control flow for functions is not analysed
        assembly {
            function g() { revert(0, 0) }
            g()
        }
    }
}
// ----
// TypeError 3464: (87-98): This variable is of storage pointer type and can be returned without prior assignment, which would lead to undefined behaviour.
